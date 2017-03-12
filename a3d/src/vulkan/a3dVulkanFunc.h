//-------------------------------------------------------------------------------------------------
// File : a3dVulkanFunc.h
// Desc : Vulkan Function Table.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

#if defined(VK_EXT_debug_marker)
extern PFN_vkDebugMarkerSetObjectTagEXT     vkDebugMarkerSetObjectTag;
extern PFN_vkDebugMarkerSetObjectNameEXT    vkDebugMarkerSetObjectName;
extern PFN_vkCmdDebugMarkerBeginEXT         vkCmdDebugMarkerBegin;
extern PFN_vkCmdDebugMarkerEndEXT           vkCmdDebugMarkerEnd;
extern PFN_vkCmdDebugMarkerInsertEXT        vkCmdDebugMarkerInsert;
#endif

#if defined(VK_KHR_push_descriptor)
extern PFN_vkCmdPushDescriptorSetKHR        vkCmdPushDescriptorSet;
#endif

#if defined(VK_EXT_hdr_metadata)
extern PFN_vkSetHdrMetadataEXT              vkSetHdrMetadata;
#endif

