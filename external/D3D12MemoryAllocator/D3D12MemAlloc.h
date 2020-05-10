//
// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

/** \mainpage D3D12 Memory Allocator

<b>Version 2.0.0-development</b> (2020-04-07)

Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved. \n
License: MIT

Documentation of all members: D3D12MemAlloc.h

\section main_table_of_contents Table of contents

- <b>User guide</b>
    - \subpage quick_start
        - [Project setup](@ref quick_start_project_setup)
        - [Creating resources](@ref quick_start_creating_resources)
        - [Mapping memory](@ref quick_start_mapping_memory)
    - \subpage reserving_memory
- \subpage configuration
  - [Custom CPU memory allocator](@ref custom_memory_allocator)
- \subpage general_considerations
  - [Thread safety](@ref general_considerations_thread_safety)
  - [Future plans](@ref general_considerations_future_plans)
  - [Features not supported](@ref general_considerations_features_not_supported)
		
\section main_see_also See also

- [Product page on GPUOpen](https://gpuopen.com/gaming-product/d3d12-memory-allocator/)
- [Source repository on GitHub](https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator)


\page quick_start Quick start

\section quick_start_project_setup Project setup and initialization

This is a small, standalone C++ library. It consists of a pair of 2 files:
"%D3D12MemAlloc.h" header file with public interface and "D3D12MemAlloc.cpp" with
internal implementation. The only external dependencies are WinAPI, Direct3D 12,
and parts of C/C++ standard library (but STL containers, exceptions, or RTTI are
not used).

The library is developed and tested using Microsoft Visual Studio 2019, but it
should work with other compilers as well. It is designed for 64-bit code.

To use the library in your project:

(1.) Copy files `D3D12MemAlloc.cpp`, `%D3D12MemAlloc.h` to your project.

(2.) Make `D3D12MemAlloc.cpp` compiling as part of the project, as C++ code.

(3.) Include library header in each CPP file that needs to use the library.

\code
#include "D3D12MemAlloc.h"
\endcode

(4.) Right after you created `ID3D12Device`, fill D3D12MA::ALLOCATOR_DESC
structure and call function D3D12MA::CreateAllocator to create the main
D3D12MA::Allocator object.

Please note that all symbols of the library are declared inside #D3D12MA namespace.

\code
IDXGIAdapter* adapter = (...)
ID3D12Device* device = (...)

D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
allocatorDesc.pDevice = device;
allocatorDesc.pAdapter = adapter;

D3D12MA::Allocator* allocator;
HRESULT hr = D3D12MA::CreateAllocator(&allocatorDesc, &allocator);
\endcode

(5.) Right before destroying the D3D12 device, destroy the allocator object.

Please note that objects of this library must be destroyed by calling `Release`
method (despite they are not COM interfaces and no reference counting is involved).

\code
allocator->Release();
\endcode


\section quick_start_creating_resources Creating resources

To use the library for creating resources (textures and buffers), call method
D3D12MA::Allocator::CreateResource in the place where you would previously call
`ID3D12Device::CreateCommittedResource`.

The function has similar syntax, but it expects structure D3D12MA::ALLOCATION_DESC
to be passed along with `D3D12_RESOURCE_DESC` and other parameters for created
resource. This structure describes parameters of the desired memory allocation,
including choice of `D3D12_HEAP_TYPE`.

The function also returns a new object of type D3D12MA::Allocation, created along
with usual `ID3D12Resource`. It represents allocated memory and can be queried
for size, offset, `ID3D12Resource`, and `ID3D12Heap` if needed.

\code
D3D12_RESOURCE_DESC resourceDesc = {};
resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
resourceDesc.Alignment = 0;
resourceDesc.Width = 1024;
resourceDesc.Height = 1024;
resourceDesc.DepthOrArraySize = 1;
resourceDesc.MipLevels = 1;
resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
resourceDesc.SampleDesc.Count = 1;
resourceDesc.SampleDesc.Quality = 0;
resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

D3D12MA::ALLOCATION_DESC allocationDesc = {};
allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

D3D12Resource* resource;
D3D12MA::Allocation* allocation;
HRESULT hr = allocator->CreateResource(
    &allocationDesc,
    &resourceDesc,
    D3D12_RESOURCE_STATE_COPY_DEST,
    NULL,
    &allocation,
    IID_PPV_ARGS(&resource));
\endcode

You need to remember both resource and allocation objects and destroy them
separately when no longer needed.

\code
allocation->Release();
resource->Release();
\endcode

The advantage of using the allocator instead of creating committed resource, and
the main purpose of this library, is that it can decide to allocate bigger memory
heap internally using `ID3D12Device::CreateHeap` and place multiple resources in
it, at different offsets, using `ID3D12Device::CreatePlacedResource`. The library
manages its own collection of allocated memory blocks (heaps) and remembers which
parts of them are occupied and which parts are free to be used for new resources.

It is important to remember that resources created as placed don't have their memory
initialized to zeros, but may contain garbage data, so they need to be fully initialized
before usage, e.g. using Clear (`ClearRenderTargetView`), Discard (`DiscardResource`),
or copy (`CopyResource`).

The library also automatically handles resource heap tier.
When `D3D12_FEATURE_DATA_D3D12_OPTIONS::ResourceHeapTier` equals `D3D12_RESOURCE_HEAP_TIER_1`,
resources of 3 types: buffers, textures that are render targets or depth-stencil,
and other textures must be kept in separate heaps. When `D3D12_RESOURCE_HEAP_TIER_2`,
they can be kept together. By using this library, you don't need to handle this
manually.


\section quick_start_mapping_memory Mapping memory

The process of getting regular CPU-side pointer to the memory of a resource in
Direct3D is called "mapping". There are rules and restrictions to this process,
as described in D3D12 documentation of [ID3D12Resource::Map method](https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/nf-d3d12-id3d12resource-map).

Mapping happens on the level of particular resources, not entire memory heaps,
and so it is out of scope of this library. Just as the linked documentation says:

- Returned pointer refers to data of particular subresource, not entire memory heap.
- You can map same resource multiple times. It is ref-counted internally.
- Mapping is thread-safe.
- Unmapping is not required before resource destruction.
- Unmapping may not be required before using written data - some heap types on
  some platforms support resources persistently mapped.

When using this library, you can map and use your resources normally without
considering whether they are created as committed resources or placed resources in one large heap.

Example for buffer created and filled in `UPLOAD` heap type:

\code
const UINT64 bufSize = 65536;
const float* bufData = (...);

D3D12_RESOURCE_DESC resourceDesc = {};
resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
resourceDesc.Alignment = 0;
resourceDesc.Width = bufSize;
resourceDesc.Height = 1;
resourceDesc.DepthOrArraySize = 1;
resourceDesc.MipLevels = 1;
resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
resourceDesc.SampleDesc.Count = 1;
resourceDesc.SampleDesc.Quality = 0;
resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

D3D12MA::ALLOCATION_DESC allocationDesc = {};
allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

D3D12Resource* resource;
D3D12MA::Allocation* allocation;
HRESULT hr = allocator->CreateResource(
    &allocationDesc,
    &resourceDesc,
    D3D12_RESOURCE_STATE_GENERIC_READ,
    NULL,
    &allocation,
    IID_PPV_ARGS(&resource));

void* mappedPtr;
hr = resource->Map(0, NULL, &mappedPtr);

memcpy(mappedPtr, bufData, bufSize);

resource->Unmap(0, NULL);
\endcode


\page reserving_memory Reserving minimum amount of memory

The library automatically allocates and frees memory heaps.
It also applies some hysteresis so that it doesn't allocate and free entire heap
when you repeatedly create and release a single resource.
However, if you want to make sure certain number of bytes is always allocated as heaps in a specific pool,
you can use functions designed for this purpose:

- For default heaps use D3D12MA::Allocator::SetDefaultHeapMinBytes.
- For custom heaps use D3D12MA::Pool::SetMinBytes.

Default is 0. You can change this parameter any time.
Setting it to higher value may cause new heaps to be allocated.
If this allocation fails, the function returns appropriate error code, but the parameter remains set to the new value.
Setting it to lower value may cause some empty heaps to be released.

You can always call D3D12MA::Allocator::SetDefaultHeapMinBytes for 3 sets of heap flags separately:
`D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS`, `D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES`, `D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES`.
When ResourceHeapTier = 2, so that all types of resourced are kept together,
these 3 values as simply summed up to calculate minimum amount of bytes for default pool with certain heap type.
Alternatively, when ResourceHeapTier = 2, you can call this function with
`D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES` = 0. This will set a single value for the default pool and
will override the sum of those three.

Reservation of minimum number of bytes interacts correctly with
D3D12MA::POOL_DESC::MinBlockCount and D3D12MA::POOL_DESC::MaxBlockCount.
For example, free blocks (heaps) of a custom pool will be released only when
their number doesn't fall below `MinBlockCount` and their sum size doesn't fall below `MinBytes`.

Some restrictions apply:

- Setting `MinBytes` doesn't interact with memory budget. The allocator tries
  to create additional heaps when necessary without checking if they will exceed the budget.
- Resources created as committed don't count into the number of bytes compared with `MinBytes` set.
  Only placed resources are considered.


\page configuration Configuration

Please check file `D3D12MemAlloc.cpp` lines between "Configuration Begin" and
"Configuration End" to find macros that you can define to change the behavior of
the library, primarily for debugging purposes.

\section custom_memory_allocator Custom CPU memory allocator

If you use custom allocator for CPU memory rather than default C++ operator `new`
and `delete` or `malloc` and `free` functions, you can make this library using
your allocator as well by filling structure D3D12MA::ALLOCATION_CALLBACKS and
passing it as optional member D3D12MA::ALLOCATOR_DESC::pAllocationCallbacks.
Functions pointed there will be used by the library to make any CPU-side
allocations. Example:

\code
#include <malloc.h>

void* CustomAllocate(size_t Size, size_t Alignment, void* pUserData)
{
    void* memory = _aligned_malloc(Size, Alignment);
    // Your extra bookkeeping here...
    return memory;
}

void CustomFree(void* pMemory, void* pUserData)
{
    // Your extra bookkeeping here...
    _aligned_free(pMemory);
}

(...)

D3D12MA::ALLOCATION_CALLBACKS allocationCallbacks = {};
allocationCallbacks.pAllocate = &CustomAllocate;
allocationCallbacks.pFree = &CustomFree;

D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
allocatorDesc.pDevice = device;
allocatorDesc.pAdapter = adapter;
allocatorDesc.pAllocationCallbacks = &allocationCallbacks;

D3D12MA::Allocator* allocator;
HRESULT hr = D3D12MA::CreateAllocator(&allocatorDesc, &allocator);
\endcode


\page general_considerations General considerations

\section general_considerations_thread_safety Thread safety

- The library has no global state, so separate D3D12MA::Allocator objects can be used independently.
  In typical applications there should be no need to create multiple such objects though - one per `ID3D12Device` is enough.
- All calls to methods of D3D12MA::Allocator class are safe to be made from multiple
  threads simultaneously because they are synchronized internally when needed.
- When the allocator is created with D3D12MA::ALLOCATOR_FLAG_SINGLETHREADED,
  calls to methods of D3D12MA::Allocator class must be made from a single thread or synchronized by the user.
  Using this flag may improve performance.

\section general_considerations_future_plans Future plans

Features planned for future releases:

Near future: feature parity with [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/), including:

- Alternative allocation algorithms: linear allocator, buddy allocator
- JSON dump that can be visualized on a picture
- Support for priorities using `ID3D12Device1::SetResidencyPriority`

Later:

- Memory defragmentation
- Support for multi-GPU (multi-adapter)

\section general_considerations_features_not_supported Features not supported

Features deliberately excluded from the scope of this library:

- Descriptor allocation. Although also called "heaps", objects that represent
  descriptors are separate part of the D3D12 API from buffers and textures.
- Support for `D3D12_HEAP_TYPE_CUSTOM`. Only the default heap types are supported:
  `UPLOAD`, `DEFAULT`, `READBACK`.
- Support for reserved (tiled) resources. We don't recommend using them.
- Support for `ID3D12Device::Evict` and `MakeResident`. We don't recommend using them.

*/

// Define this macro to 0 to disable usage of DXGI 1.4 (needed for IDXGIAdapter3 and query for memory budget).
#ifndef D3D12MA_DXGI_1_4
    #define D3D12MA_DXGI_1_4 1
#endif

// If using this library on a platform different than Windows PC, you should
// include D3D12-compatible header before this library on your own and define this macro.
#ifndef D3D12MA_D3D12_HEADERS_ALREADY_INCLUDED
    #include <d3d12.h>
    #include <dxgi.h>
#endif

/*
When defined to value other than 0, the library will try to use
D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT or D3D12_SMALL_MSAA_RESOURCE_PLACEMENT_ALIGNMENT
for created textures when possible, which can save memory because some small textures
may get their alignment 4K and their size a multiply of 4K instead of 64K.

#define D3D12MA_USE_SMALL_RESOURCE_PLACEMENT_ALIGNMENT 0
    Disables small texture alignment.
#define D3D12MA_USE_SMALL_RESOURCE_PLACEMENT_ALIGNMENT 1
    Enables conservative algorithm that will use small alignment only for some textures
    that are surely known to support it.
#define D3D12MA_USE_SMALL_RESOURCE_PLACEMENT_ALIGNMENT 2
    Enables query for small alignment to D3D12 (based on Microsoft sample) which will
    enable small alignment for more textures, but will also generate D3D Debug Layer
    error #721 on call to ID3D12Device::GetResourceAllocationInfo, which you should just
    ignore.
*/
#ifndef D3D12MA_USE_SMALL_RESOURCE_PLACEMENT_ALIGNMENT
    #define D3D12MA_USE_SMALL_RESOURCE_PLACEMENT_ALIGNMENT 1
#endif

/// \cond INTERNAL

#define D3D12MA_CLASS_NO_COPY(className) \
    private: \
        className(const className&) = delete; \
        className(className&&) = delete; \
        className& operator=(const className&) = delete; \
        className& operator=(className&&) = delete;

// To be used with MAKE_HRESULT to define custom error codes.
#define FACILITY_D3D12MA 3542

/// \endcond

namespace D3D12MA
{

/// \cond INTERNAL
class AllocatorPimpl;
class PoolPimpl;
class NormalBlock;
class BlockVector;
class JsonWriter;
/// \endcond

class Pool;
class Allocator;
struct StatInfo;

/// Pointer to custom callback function that allocates CPU memory.
typedef void* (*ALLOCATE_FUNC_PTR)(size_t Size, size_t Alignment, void* pUserData);
/**
\brief Pointer to custom callback function that deallocates CPU memory.

`pMemory = null` should be accepted and ignored.
*/
typedef void (*FREE_FUNC_PTR)(void* pMemory, void* pUserData);

/// Custom callbacks to CPU memory allocation functions.
struct ALLOCATION_CALLBACKS
{
    /// %Allocation function.
    ALLOCATE_FUNC_PTR pAllocate;
    /// Dellocation function.
    FREE_FUNC_PTR pFree;
    /// Custom data that will be passed to allocation and deallocation functions as `pUserData` parameter.
    void* pUserData;
};

/// \brief Bit flags to be used with ALLOCATION_DESC::Flags.
typedef enum ALLOCATION_FLAGS
{
    /// Zero
    ALLOCATION_FLAG_NONE = 0,

    /**
    Set this flag if the allocation should have its own dedicated memory allocation (committed resource with implicit heap).
    
    Use it for special, big resources, like fullscreen textures used as render targets.
    */
    ALLOCATION_FLAG_COMMITTED = 0x1,

    /**
    Set this flag to only try to allocate from existing memory heaps and never create new such heap.

    If new allocation cannot be placed in any of the existing heaps, allocation
    fails with `E_OUTOFMEMORY` error.

    You should not use D3D12MA::ALLOCATION_FLAG_COMMITTED and
    D3D12MA::ALLOCATION_FLAG_NEVER_ALLOCATE at the same time. It makes no sense.
    */
    ALLOCATION_FLAG_NEVER_ALLOCATE = 0x2,

    /** Create allocation only if additional memory required for it, if any, won't exceed
    memory budget. Otherwise return `E_OUTOFMEMORY`.
    */
    ALLOCATION_FLAG_WITHIN_BUDGET = 0x4,
} ALLOCATION_FLAGS;

/// \brief Parameters of created D3D12MA::Allocation object. To be used with Allocator::CreateResource.
struct ALLOCATION_DESC
{
    /// Flags.
    ALLOCATION_FLAGS Flags;
    /** \brief The type of memory heap where the new allocation should be placed.

    It must be one of: `D3D12_HEAP_TYPE_DEFAULT`, `D3D12_HEAP_TYPE_UPLOAD`, `D3D12_HEAP_TYPE_READBACK`.

    When D3D12MA::ALLOCATION_DESC::CustomPool != NULL this member is ignored.
    */
    D3D12_HEAP_TYPE HeapType;
    /** \brief Additional heap flags to be used when allocating memory.

    In most cases it can be 0.
    
    - If you use D3D12MA::Allocator::CreateResource(), you don't need to care.
      Necessary flag `D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS`, `D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES`,
      or `D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES` is added automatically.
    - If you use D3D12MA::Allocator::AllocateMemory(), you should specify one of those `ALLOW_ONLY` flags.
      Except when you validate that D3D12MA::Allocator::GetD3D12Options()`.ResourceHeapTier == D3D12_RESOURCE_HEAP_TIER_1` -
      then you can leave it 0.
    - You can specify additional flags if needed. Then the memory will always be allocated as
      separate block using `D3D12Device::CreateCommittedResource` or `CreateHeap`, not as part of an existing larget block.

    When D3D12MA::ALLOCATION_DESC::CustomPool != NULL this member is ignored.
    */
    D3D12_HEAP_FLAGS ExtraHeapFlags;
    /** \brief Custom pool to place the new resource in. Optional.

    When not NULL, the resource will be created inside specified custom pool.
    It will then never be created as committed.
    */
    Pool* CustomPool;
};

/** \brief Represents single memory allocation.

It may be either implicit memory heap dedicated to a single resource or a
specific region of a bigger heap plus unique offset.

To create such object, fill structure D3D12MA::ALLOCATION_DESC and call function
Allocator::CreateResource.

The object remembers size and some other information.
To retrieve this information, use methods of this class.

The object also remembers `ID3D12Resource` and "owns" a reference to it,
so it calls `Release()` on the resource when destroyed.
*/
class Allocation
{
public:
    /** \brief Deletes this object.

    This function must be used instead of destructor, which is private.
    There is no reference counting involved.
    */
    void Release();

    /** \brief Returns offset in bytes from the start of memory heap.

    If the Allocation represents committed resource with implicit heap, returns 0.
    */
    UINT64 GetOffset() const;

    /** \brief Returns size in bytes of the resource.

    Works also with committed resources.
    */
    UINT64 GetSize() const { return m_Size; }

    /** \brief Returns D3D12 resource associated with this object.

    Calling this method doesn't increment resource's reference counter.
    */
    ID3D12Resource* GetResource() const { return m_Resource; }

    /** \brief Returns memory heap that the resource is created in.

    If the Allocation represents committed resource with implicit heap, returns NULL.
    */
    ID3D12Heap* GetHeap() const;

    /** \brief Associates a name with the allocation object. This name is for use in debug diagnostics and tools.

    Internal copy of the string is made, so the memory pointed by the argument can be
    changed of freed immediately after this call.

    `Name` can be null.
    */
    void SetName(LPCWSTR Name);

    /** \brief Returns the name associated with the allocation object.

    Returned string points to an internal copy.

    If no name was associated with the allocation, returns null.
    */
    LPCWSTR GetName() const { return m_Name; }

    /** \brief Returns `TRUE` if the memory of the allocation was filled with zeros when the allocation was created.

    Returns `TRUE` only if the allocator is sure that the entire memory where the
    allocation was created was filled with zeros at the moment the allocation was made.
    
    Returns `FALSE` if the memory could potentially contain garbage data.
    If it's a render-target or depth-stencil texture, it then needs proper
    initialization with `ClearRenderTargetView`, `ClearDepthStencilView`, `DiscardResource`,
    or a copy operation, as described on page:
    [ID3D12Device::CreatePlacedResource method - Notes on the required resource initialization](https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-createplacedresource#notes-on-the-required-resource-initialization).
    Please note that rendering a fullscreen triangle or quad to the texture as
    a render target is not a proper way of initialization!

    See also articles:
    ["Coming to DirectX 12: More control over memory allocation"](https://devblogs.microsoft.com/directx/coming-to-directx-12-more-control-over-memory-allocation/),
    ["Initializing DX12 Textures After Allocation and Aliasing"](https://asawicki.info/news_1724_initializing_dx12_textures_after_allocation_and_aliasing).
    */
    BOOL WasZeroInitialized() const { return m_PackedData.WasZeroInitialized(); }

private:
    friend class AllocatorPimpl;
    friend class BlockVector;
    friend class JsonWriter;
    template<typename T> friend void D3D12MA_DELETE(const ALLOCATION_CALLBACKS&, T*);
    template<typename T> friend class PoolAllocator;

    enum Type
    {
        TYPE_COMMITTED,
        TYPE_PLACED,
        TYPE_HEAP,
        TYPE_COUNT
    };

    AllocatorPimpl* m_Allocator;
    UINT64 m_Size;
    ID3D12Resource* m_Resource;
    UINT m_CreationFrameIndex;
    wchar_t* m_Name;

    union
    {
        struct
        {
            D3D12_HEAP_TYPE heapType;
        } m_Committed;

        struct
        {
            UINT64 offset;
            NormalBlock* block;
        } m_Placed;

        struct
        {
            D3D12_HEAP_TYPE heapType;
            ID3D12Heap* heap;
        } m_Heap;
    };

    struct PackedData
    {
    public:
        PackedData() :
            m_Type(0), m_ResourceDimension(0), m_ResourceFlags(0), m_TextureLayout(0), m_WasZeroInitialized(0) { }

        Type GetType() const { return (Type)m_Type; }
        D3D12_RESOURCE_DIMENSION GetResourceDimension() const { return (D3D12_RESOURCE_DIMENSION)m_ResourceDimension; }
        D3D12_RESOURCE_FLAGS GetResourceFlags() const { return (D3D12_RESOURCE_FLAGS)m_ResourceFlags; }
        D3D12_TEXTURE_LAYOUT GetTextureLayout() const { return (D3D12_TEXTURE_LAYOUT)m_TextureLayout; }
        BOOL WasZeroInitialized() const { return (BOOL)m_WasZeroInitialized; }

        void SetType(Type type);
        void SetResourceDimension(D3D12_RESOURCE_DIMENSION resourceDimension);
        void SetResourceFlags(D3D12_RESOURCE_FLAGS resourceFlags);
        void SetTextureLayout(D3D12_TEXTURE_LAYOUT textureLayout);
        void SetWasZeroInitialized(BOOL wasZeroInitialized) { m_WasZeroInitialized = wasZeroInitialized ? 1 : 0; }

    private:
        UINT m_Type : 2;               // enum Type
        UINT m_ResourceDimension : 3;  // enum D3D12_RESOURCE_DIMENSION
        UINT m_ResourceFlags : 24;     // flags D3D12_RESOURCE_FLAGS
        UINT m_TextureLayout : 9;      // enum D3D12_TEXTURE_LAYOUT
        UINT m_WasZeroInitialized : 1; // BOOL
    } m_PackedData;

    Allocation(AllocatorPimpl* allocator, UINT64 size, BOOL wasZeroInitialized);
    ~Allocation();
    void InitCommitted(D3D12_HEAP_TYPE heapType);
    void InitPlaced(UINT64 offset, UINT64 alignment, NormalBlock* block);
    void InitHeap(D3D12_HEAP_TYPE heapType, ID3D12Heap* heap);
    void SetResource(ID3D12Resource* resource, const D3D12_RESOURCE_DESC* pResourceDesc);
    void FreeName();

    D3D12MA_CLASS_NO_COPY(Allocation)
};

/// \brief Parameters of created D3D12MA::Pool object. To be used with D3D12MA::Allocator::CreatePool.
struct POOL_DESC
{
    /** \brief The type of memory heap where allocations of this pool should be placed.

    It must be one of: `D3D12_HEAP_TYPE_DEFAULT`, `D3D12_HEAP_TYPE_UPLOAD`, `D3D12_HEAP_TYPE_READBACK`.
    */
    D3D12_HEAP_TYPE HeapType;
    /** \brief Heap flags to be used when allocating heaps of this pool.

    It should contain one of these values, depending on type of resources you are going to create in this heap:
    `D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS`,
    `D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES`,
    `D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES`.
    Except if ResourceHeapTier = 2, then it may be `D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES` = 0.
    
    You can specify additional flags if needed.
    */
    D3D12_HEAP_FLAGS HeapFlags;
    /** \brief Size of a single heap (memory block) to be allocated as part of this pool, in bytes. Optional.

    Specify nonzero to set explicit, constant size of memory blocks used by this pool.
    Leave 0 to use default and let the library manage block sizes automatically.
    Then sizes of particular blocks may vary.
    */
    UINT64 BlockSize;
    /** \brief Minimum number of heaps (memory blocks) to be always allocated in this pool, even if they stay empty. Optional.

    Set to 0 to have no preallocated blocks and allow the pool be completely empty.
    */
    UINT MinBlockCount;
    /** \brief Maximum number of heaps (memory blocks) that can be allocated in this pool. Optional.

    Set to 0 to use default, which is `UINT64_MAX`, which means no limit.

    Set to same value as D3D12MA::POOL_DESC::MinBlockCount to have fixed amount of memory allocated
    throughout whole lifetime of this pool.
    */
    UINT MaxBlockCount;
};

/** \brief Custom memory pool

Represents a separate set of heaps (memory blocks) that can be used to create
D3D12MA::Allocation-s and resources in it. Usually there is no need to create custom
pools - creating resources in default pool is sufficient.

To create custom pool, fill D3D12MA::POOL_DESC and call D3D12MA::Allocator::CreatePool.
*/
class Pool
{
public:
    /** \brief Deletes pool object, frees D3D12 heaps (memory blocks) managed by it. Allocations and resources must already be released!

    It doesn't delete allocations and resources created in this pool. They must be all
    released before calling this function!
    */
    void Release();
    
    /** \brief Returns copy of parameters of the pool.

    These are the same parameters as passed to D3D12MA::Allocator::CreatePool.
    */
    POOL_DESC GetDesc() const;

    /** \brief Sets the minimum number of bytes that should always be allocated (reserved) in this pool.

    See also: \subpage reserving_memory.
    */
    HRESULT SetMinBytes(UINT64 minBytes);

    /** \brief Retrieves statistics from the current state of this pool.
    */
    void CalculateStats(StatInfo* pStats);

    /** \brief Associates a name with the pool. This name is for use in debug diagnostics and tools.

    Internal copy of the string is made, so the memory pointed by the argument can be
    changed of freed immediately after this call.

    `Name` can be NULL.
    */
    void SetName(LPCWSTR Name);

    /** \brief Returns the name associated with the pool object.

    Returned string points to an internal copy.

    If no name was associated with the allocation, returns NULL.
    */
    LPCWSTR GetName() const;

private:
    friend class Allocator;
    friend class AllocatorPimpl;
    template<typename T> friend void D3D12MA_DELETE(const ALLOCATION_CALLBACKS&, T*);

    PoolPimpl* m_Pimpl;

    Pool(Allocator* allocator, const POOL_DESC &desc);
    ~Pool();

    D3D12MA_CLASS_NO_COPY(Pool)
};

/// \brief Bit flags to be used with ALLOCATOR_DESC::Flags.
typedef enum ALLOCATOR_FLAGS
{
    /// Zero
    ALLOCATOR_FLAG_NONE = 0,

    /**
    Allocator and all objects created from it will not be synchronized internally,
    so you must guarantee they are used from only one thread at a time or
    synchronized by you.

    Using this flag may increase performance because internal mutexes are not used.
    */
    ALLOCATOR_FLAG_SINGLETHREADED = 0x1,

    /**
    Every allocation will have its own memory block.
    To be used for debugging purposes.
   */
    ALLOCATOR_FLAG_ALWAYS_COMMITTED = 0x2,
} ALLOCATOR_FLAGS;

/// \brief Parameters of created Allocator object. To be used with CreateAllocator().
struct ALLOCATOR_DESC
{
    /// Flags.
    ALLOCATOR_FLAGS Flags;
    
    /** Direct3D device object that the allocator should be attached to.

    Allocator is doing `AddRef`/`Release` on this object.
    */
    ID3D12Device* pDevice;
    
    /** \brief Preferred size of a single `ID3D12Heap` block to be allocated.
    
    Set to 0 to use default, which is currently 256 MiB.
    */
    UINT64 PreferredBlockSize;
    
    /** \brief Custom CPU memory allocation callbacks. Optional.

    Optional, can be null. When specified, will be used for all CPU-side memory allocations.
    */
    const ALLOCATION_CALLBACKS* pAllocationCallbacks;

    /** DXGI Adapter object that you use for D3D12 and this allocator.

    Allocator is doing `AddRef`/`Release` on this object.
    */
    IDXGIAdapter* pAdapter;
};

/**
\brief Number of D3D12 memory heap types supported.
*/
const UINT HEAP_TYPE_COUNT = 3;

/**
\brief Calculated statistics of memory usage in entire allocator.
*/
struct StatInfo
{
    /// Number of memory blocks (heaps) allocated.
    UINT BlockCount;
    /// Number of D3D12MA::Allocation objects allocated.
    UINT AllocationCount;
    /// Number of free ranges of memory between allocations.
    UINT UnusedRangeCount;
    /// Total number of bytes occupied by all allocations.
    UINT64 UsedBytes;
    /// Total number of bytes occupied by unused ranges.
    UINT64 UnusedBytes;
    UINT64 AllocationSizeMin;
    UINT64 AllocationSizeAvg;
    UINT64 AllocationSizeMax;
    UINT64 UnusedRangeSizeMin;
    UINT64 UnusedRangeSizeAvg;
    UINT64 UnusedRangeSizeMax;
};

/**
\brief General statistics from the current state of the allocator.
*/
struct Stats
{
    /// Total statistics from all heap types.
    StatInfo Total;
    /**
    One StatInfo for each type of heap located at the following indices:
    0 - DEFAULT, 1 - UPLOAD, 2 - READBACK.
    */
    StatInfo HeapType[HEAP_TYPE_COUNT];
};

/** \brief Statistics of current memory usage and available budget, in bytes, for GPU or CPU memory.
*/
struct Budget
{
    /** \brief Sum size of all memory blocks allocated from particular heap type, in bytes.
    */
    UINT64 BlockBytes;

    /** \brief Sum size of all allocations created in particular heap type, in bytes.

    Always less or equal than `BlockBytes`.
    Difference `BlockBytes - AllocationBytes` is the amount of memory allocated but unused -
    available for new allocations or wasted due to fragmentation.
    */
    UINT64 AllocationBytes;

    /** \brief Estimated current memory usage of the program, in bytes.

    Fetched from system using `IDXGIAdapter3::QueryVideoMemoryInfo` if enabled.

    It might be different than `BlockBytes` (usually higher) due to additional implicit objects
    also occupying the memory, like swapchain, pipeline state objects, descriptor heaps, command lists, or
    memory blocks allocated outside of this library, if any.
    */
    UINT64 UsageBytes;

    /** \brief Estimated amount of memory available to the program, in bytes.

    Fetched from system using `IDXGIAdapter3::QueryVideoMemoryInfo` if enabled.

    It might be different (most probably smaller) than memory sizes reported in `DXGI_ADAPTER_DESC` due to factors
    external to the program, like other programs also consuming system resources.
    Difference `BudgetBytes - UsageBytes` is the amount of additional memory that can probably
    be allocated without problems. Exceeding the budget may result in various problems.
    */
    UINT64 BudgetBytes;
};

/**
\brief Represents main object of this library initialized for particular `ID3D12Device`.

Fill structure D3D12MA::ALLOCATOR_DESC and call function CreateAllocator() to create it.
Call method Allocator::Release to destroy it.

It is recommended to create just one object of this type per `ID3D12Device` object,
right after Direct3D 12 is initialized and keep it alive until before Direct3D device is destroyed.
*/
class Allocator
{
public:
    /** \brief Deletes this object.
    
    This function must be used instead of destructor, which is private.
    There is no reference counting involved.
    */
    void Release();
    
    /// Returns cached options retrieved from D3D12 device.
    const D3D12_FEATURE_DATA_D3D12_OPTIONS& GetD3D12Options() const;

    /** \brief Allocates memory and creates a D3D12 resource (buffer or texture). This is the main allocation function.

    The function is similar to `ID3D12Device::CreateCommittedResource`, but it may
    really call `ID3D12Device::CreatePlacedResource` to assign part of a larger,
    existing memory heap to the new resource, which is the main purpose of this
    whole library.

    If `ppvResource` is null, you receive only `ppAllocation` object from this function.
    It holds pointer to `ID3D12Resource` that can be queried using function D3D12::Allocation::GetResource().
    Reference count of the resource object is 1.
    It is automatically destroyed when you destroy the allocation object.

    If 'ppvResource` is not null, you receive pointer to the resource next to allocation object.
    Reference count of the resource object is then 2, so you need to manually `Release` it
    along with the allocation.

    \param pAllocDesc   Parameters of the allocation.
    \param pResourceDesc   Description of created resource.
    \param InitialResourceState   Initial resource state.
    \param pOptimizedClearValue   Optional. Either null or optimized clear value.
    \param[out] ppAllocation   Filled with pointer to new allocation object created.
    \param riidResource   IID of a resource to be created. Must be `__uuidof(ID3D12Resource)`.
    \param[out] ppvResource   Optional. If not null, filled with pointer to new resouce created.
    */
    HRESULT CreateResource(
        const ALLOCATION_DESC* pAllocDesc,
        const D3D12_RESOURCE_DESC* pResourceDesc,
        D3D12_RESOURCE_STATES InitialResourceState,
        const D3D12_CLEAR_VALUE *pOptimizedClearValue,
        Allocation** ppAllocation,
        REFIID riidResource,
        void** ppvResource);

    /** \brief Allocates memory without creating any resource placed in it.

    This function is similar to `ID3D12Device::CreateHeap`, but it may really assign
    part of a larger, existing heap to the allocation.

    `pAllocDesc->heapFlags` should contain one of these values, depending on type of resources you are going to create in this memory:
    `D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS`,
    `D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES`,
    `D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES`.
    Except if you validate that ResourceHeapTier = 2 - then `heapFlags`
    may be `D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES` = 0.
    Additional flags in `heapFlags` are allowed as well.

    `pAllocInfo->SizeInBytes` must be multiply of 64KB.
    `pAllocInfo->Alignment` must be one of the legal values as described in documentation of `D3D12_HEAP_DESC`.

    If you use D3D12MA::ALLOCATION_FLAG_COMMITTED you will get a separate memory block -
    a heap that always has offset 0.
    */
    HRESULT AllocateMemory(
        const ALLOCATION_DESC* pAllocDesc,
        const D3D12_RESOURCE_ALLOCATION_INFO* pAllocInfo,
        Allocation** ppAllocation);

    /** \brief Creates a new resource in place of an existing allocation. This is useful for memory aliasing.

    \param pAllocation Existing allocation indicating the memory where the new resource should be created.
        It can be created using D3D12MA::Allocator::CreateResource and already have a resource bound to it,
        or can be a raw memory allocated with D3D12MA::Allocator::AllocateMemory.
        It must not be created as committed so that `ID3D12Heap` is available and not implicit.
    \param AllocationLocalOffset Additional offset in bytes to be applied when allocating the resource.
        Local from the start of `pAllocation`, not the beginning of the whole `ID3D12Heap`!
        If the new resource should start from the beginning of the `pAllocation` it should be 0.
    \param pResourceDesc Description of the new resource to be created.
    \param InitialResourceState
    \param pOptimizedClearValue
    \param riidResource
    \param[out] ppvResource Returns pointer to the new resource.
        The resource is not bound with `pAllocation`.
        This pointer must not be null - you must get the resource pointer and `Release` it when no longer needed.

    Memory requirements of the new resource are checked for validation.
    If its size exceeds the end of `pAllocation` or required alignment is not fulfilled
    considering `pAllocation->GetOffset() + AllocationLocalOffset`, the function
    returns `E_INVALIDARG`.
    */
    HRESULT CreateAliasingResource(
        Allocation* pAllocation,
        UINT64 AllocationLocalOffset,
        const D3D12_RESOURCE_DESC* pResourceDesc,
        D3D12_RESOURCE_STATES InitialResourceState,
        const D3D12_CLEAR_VALUE *pOptimizedClearValue,
        REFIID riidResource,
        void** ppvResource);

    /** \brief Creates custom pool.
    */
    HRESULT CreatePool(
        const POOL_DESC* pPoolDesc,
        Pool** ppPool);

    /** \brief Sets the minimum number of bytes that should always be allocated (reserved) in a specific default pool.

    \param heapType Must be one of: `D3D12_HEAP_TYPE_DEFAULT`, `D3D12_HEAP_TYPE_UPLOAD`, `D3D12_HEAP_TYPE_READBACK`.
    \param heapFlags Must be one of: `D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS`, `D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES`,
        `D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES`. If ResourceHeapTier = 2, it can also be `D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES`.
    \param minBytes Minimum number of bytes to keep allocated.

    See also: \subpage reserving_memory.
    */
    HRESULT SetDefaultHeapMinBytes(
        D3D12_HEAP_TYPE heapType,
        D3D12_HEAP_FLAGS heapFlags,
        UINT64 minBytes);

    /** \brief Sets the index of the current frame.

    This function is used to set the frame index in the allocator when a new game frame begins.
    */
    void SetCurrentFrameIndex(UINT frameIndex);

    /** \brief Retrieves statistics from the current state of the allocator.
    */
    void CalculateStats(Stats* pStats);

    /** \brief Retrieves information about current memory budget.

    \param[out] pGpuBudget Optional, can be null.
    \param[out] pCpuBudget Optional, can be null.

    This function is called "get" not "calculate" because it is very fast, suitable to be called
    every frame or every allocation. For more detailed statistics use CalculateStats().

    Note that when using allocator from multiple threads, returned information may immediately
    become outdated.
    */
    void GetBudget(Budget* pGpuBudget, Budget* pCpuBudget);

    /// Builds and returns statistics as a string in JSON format.
    /** @param[out] ppStatsString Must be freed using Allocator::FreeStatsString.
    @param DetailedMap `TRUE` to include full list of allocations (can make the string quite long), `FALSE` to only return statistics.
    */
    void BuildStatsString(WCHAR** ppStatsString, BOOL DetailedMap);

    /// Frees memory of a string returned from Allocator::BuildStatsString.
    void FreeStatsString(WCHAR* pStatsString);

private:
    friend HRESULT CreateAllocator(const ALLOCATOR_DESC*, Allocator**);
    template<typename T> friend void D3D12MA_DELETE(const ALLOCATION_CALLBACKS&, T*);
    friend class Pool;

    Allocator(const ALLOCATION_CALLBACKS& allocationCallbacks, const ALLOCATOR_DESC& desc);
    ~Allocator();
    
    AllocatorPimpl* m_Pimpl;
    
    D3D12MA_CLASS_NO_COPY(Allocator)
};

/** \brief Creates new main Allocator object and returns it through `ppAllocator`.

You normally only need to call it once and keep a single Allocator object for your `ID3D12Device`.
*/
HRESULT CreateAllocator(const ALLOCATOR_DESC* pDesc, Allocator** ppAllocator);

} // namespace D3D12MA

/// \cond INTERNAL
DEFINE_ENUM_FLAG_OPERATORS(D3D12MA::ALLOCATION_FLAGS);
DEFINE_ENUM_FLAG_OPERATORS(D3D12MA::ALLOCATOR_FLAGS);
/// \endcond
