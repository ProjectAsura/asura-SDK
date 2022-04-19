//-------------------------------------------------------------------------------------------------
// File : a3dVulkanFunc.h
// Desc : Vulkan Function Table.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

#if defined(VK_KHR_swapchain)
extern PFN_vkCreateSwapchainKHR                     vkCreateSwapchain;
extern PFN_vkDestroySwapchainKHR                    vkDestroySwapchain;
extern PFN_vkGetSwapchainImagesKHR                  vkGetSwapchainImages;
extern PFN_vkAcquireNextImageKHR                    vkAcquireNextImage;
extern PFN_vkQueuePresentKHR                        vkQueuePresent;
extern PFN_vkGetDeviceGroupPresentCapabilitiesKHR   vkGetDeviceGroupPresentCapabilities;
extern PFN_vkGetDeviceGroupSurfacePresentModesKHR   vkGetDeviceGroupSurfacePresentModes;
extern PFN_vkGetPhysicalDevicePresentRectanglesKHR  vkGetPhysicalDevicePresentRectangles;
extern PFN_vkAcquireNextImage2KHR                   vkAcquireNextImage2;
#endif

#if defined(VK_EXT_debug_marker)
extern PFN_vkDebugMarkerSetObjectTagEXT             vkDebugMarkerSetObjectTag;
extern PFN_vkDebugMarkerSetObjectNameEXT            vkDebugMarkerSetObjectName;
extern PFN_vkCmdDebugMarkerBeginEXT                 vkCmdDebugMarkerBegin;
extern PFN_vkCmdDebugMarkerEndEXT                   vkCmdDebugMarkerEnd;
extern PFN_vkCmdDebugMarkerInsertEXT                vkCmdDebugMarkerInsert;
#endif

#if defined(VK_KHR_push_descriptor)
extern PFN_vkCmdPushDescriptorSetKHR                vkCmdPushDescriptorSet;
#endif

#if defined(VK_EXT_hdr_metadata)
extern PFN_vkSetHdrMetadataEXT                      vkSetHdrMetadata;
#endif

#if defined(VK_NV_mesh_shader)
extern PFN_vkCmdDrawMeshTasksNV                     vkCmdDrawMeshTasks;
extern PFN_vkCmdDrawMeshTasksIndirectNV             vkCmdDrawMeshTasksIndirect;
extern PFN_vkCmdDrawMeshTasksIndirectCountNV        vkCmdDrawMeshTasksIndirectCount;
#endif

//#if defined(VK_KHR_dynamic_rendering)
//extern PFN_vkCmdBeginRenderingKHR                   vkCmdBeginRendering;
//extern PFN_vkCmdEndRenderingKHR                     vkCmdEndRendering;
//#endif