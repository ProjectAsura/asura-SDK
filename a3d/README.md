# a3d
Asura Rendering Hardware Interface Library  
This project is WIP  

## About
a3d はDirect3D12とVulkanをラップするための抽象化3Dグラフィックスライブラリです。  
主にゲームエンジンやツール作成するための足回りとして利用することを目的として開発を進めています。  
本ライブラリはMITライセンスです。  
[License](./LICENSE "License")  

## Build  
ビルドするには予め以下のものをインストールしておく必要があります。
* Visual Studio 2015 Update 3  
* Windows SDK  
* Vulkan SDK  

ビルド方法についてですが，project/build.batを実行することで，binフォルダにスタティックライブラリが生成されます。  


## Integration  
組み込みするためには，次の2つの手順が必要です。
* ヘッダファイルをインクルードする  
* スタティックライブラリをリンクする  

ヘッダファイルは includeフォルダにあります。プログラムで，次のようにa3d.hファイルをインクルードします。  
```cpp
#include <a3d.h>
```

スタティックライブラリは，binフォルダ下にDirect3D12版とVulkan版がそれぞれ別に存在します。  
ご使用したい環境に合わせてリンクするファイルを選択してください。  
Direct3D12版を使用する場合は次のファイルをリンクする必要があります。  

* a3d_d3d12d.lib (デバッグ版)
* a3d_d3d12.lib  (リリース版)
* d3d12.lib
* dxgi.lib

Vulkan版を使用する場合は次のファイルをリンクする必要があります。  

* a3d_vkd.lib (デバッグ版)
* a3d_vk.lib  (リリース版)
* vulkan-1.lib 

sampleフォルダ下に組み込み例があるので，組み込み方法が分からない場合は参考にしてください。  

### 初期化処理
まずは，ライブラリにメモリアロケータを渡す必要があります。メモリアロケータはa3d::IAllocatorインタフェースを継承してユーザーが実装する必要があります。単純な実装例は次のようになります。  
```cpp
class Allocator : a3d::IAllocator
{
public:
    void* Alloc(size_t size, size_t alignment) noexcept override
    {
        auto allocSize = a3d::RoundUp(size, alignment);
        return malloc(size);
    }

    void* Realloc(void* ptr, size_t size, size_t alignment) noexcept override
    {
        auto allocSize = a3d::RoundUp(size, alignment);
        return realloc(ptr, allocSize);
    }

    void Free(void* ptr) noexcept override
    { free(ptr); }

} g_Allocator;

```

メモリアロケータはa3d::InitSystem()メソッドを使って，ライブラリに渡します。実装例は次の通りです。

```cpp
// グラフィックスシステムの初期化.
if (!a3d::InitSystem(reinterpret_cast<a3d::IAllocator*>(&g_Allocator)))
{ return false; }
```

### 終了処理
a3dライブラリを終了させる前に，全てのオブジェクトを破棄しておく必要があります。オブジェクトの破棄はa3d::IReference::Release()メソッドを呼び出すことにより解放処理が実行されます。  
全てのオブジェクトが破棄されている状態で，a3d::TermSystem()メソッドを呼び出しを行います。このメソッドを呼び出すことで，a3d::InitSystem()メソッドで登録したメモリアロケータがnullptrに設定されます。  
```cpp
// グラフィックスシステムの終了処理.
a3d::TermSystem();
```
a3d::TermSystem()呼び出し以後は，a3dライブラリのメソッドを呼び出しを行うと不定動作を起こす可能性がありますので呼び出しないようにお願い致します。  


## Samples
sampleフォルダ下にa3dライブラリを使ったサンプルがあります。  

* [ClearColor](./sample/001_ClearColor/code "ClearColor")  
画面をクリアするサンプルです。  
![ClearColor](./doc/images/001_ClearColor.png)  

* [DrawPolygon](./sample/002_DrawPolygon/code "DrawPolygon")  
ポリゴンを描画するサンプルです。  
![DrawPolygon](./doc/images/002_DrawPolygon.png)  

* [DrawIndexed](./sample/003_DrawIndexed/code "DrawIndexed")  
インデックスバッファを用いてポリゴンを描画するサンプルです。  
![DrawIndexed](./doc/images/003_DrawIndexed.png)  

* [ConstantBuffer](./sample/004_ConstantBuffer/code "ConstantBuffer")  
定数バッファを用いてポリゴンを回転させるサンプルです。  
![ConstantBuffer](./doc/images/004_ConstantBuffer.png)  

* [DepthBuffer](./sample/005_DepthBuffer/code "DepthBuffer")  
深度バッファを用いて手前と奥のポリゴンを表示するサンプルです。  
![DepthBuffer](./doc/images/005_DepthBuffer.png)  

* [DrawTexture](./sample/006_DrawTexture/code "DrawTexture")  
テクスチャを描画するサンプルです。  
![DrawTexture](./doc/images/006_DrawTexture.png)  

* [ImGuiSample](./sample/007_ImGuiSample/code "ImGuiSample")  
ImGuiを用いたサンプルです。  
![ImGuiSample](./doc/images/007_ImGuiSample.png)  

* [RenderingTexture](./sample/008_RenderingTexture/code "RenderingTexture")  
レンダリングテクスチャを表示するサンプルです。  
![RenderingTexture](./doc/images/008_RenderingTexture.png)  


## Documents
docフォルダ下にAPIリファレンスがあります。  
* [API Document](./doc/a3d_api.chm)  


## License
This software is licensed under the MIT License, see LICENSE for more information.

