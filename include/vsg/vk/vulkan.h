#pragma once
// clang-format off

/* <editor-fold desc="Apache-2.0 License">

** Copyright (c) 2015-2020 The Khronos Group Inc.
**
** SPDX-License-Identifier: Apache-2.0

</editor-fold> */

//
// Include Vulkan headers that are available in the system.
//
#include <vulkan/vulkan.h>
//
// Then add definitions not provided by older headers below.
//

// Workaround for cppcheck
#ifndef VK_DEFINE_NON_DISPATCHABLE_HANDLE
    #if (VK_USE_64_BIT_PTR_DEFINES==1)
        #define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;
    #else
        #define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
    #endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definitions not provided prior to 1.1.77
//
#if VK_HEADER_VERSION < 77
#    define VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME "VK_EXT_descriptor_indexing"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definitions not provided prior to 1.1.82
//
#if VK_HEADER_VERSION < 82
#    define VK_KHR_create_renderpass2 1
#    define VK_KHR_CREATE_RENDERPASS_2_SPEC_VERSION 1
#    define VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME "VK_KHR_create_renderpass2"

typedef struct VkAttachmentDescription2KHR
{
    VkStructureType sType;
    const void* pNext;
    VkAttachmentDescriptionFlags flags;
    VkFormat format;
    VkSampleCountFlagBits samples;
    VkAttachmentLoadOp loadOp;
    VkAttachmentStoreOp storeOp;
    VkAttachmentLoadOp stencilLoadOp;
    VkAttachmentStoreOp stencilStoreOp;
    VkImageLayout initialLayout;
    VkImageLayout finalLayout;
} VkAttachmentDescription2KHR;

typedef struct VkAttachmentReference2KHR
{
    VkStructureType sType;
    const void* pNext;
    uint32_t attachment;
    VkImageLayout layout;
    VkImageAspectFlags aspectMask;
} VkAttachmentReference2KHR;

typedef struct VkSubpassDescription2KHR
{
    VkStructureType sType;
    const void* pNext;
    VkSubpassDescriptionFlags flags;
    VkPipelineBindPoint pipelineBindPoint;
    uint32_t viewMask;
    uint32_t inputAttachmentCount;
    const VkAttachmentReference2KHR* pInputAttachments;
    uint32_t colorAttachmentCount;
    const VkAttachmentReference2KHR* pColorAttachments;
    const VkAttachmentReference2KHR* pResolveAttachments;
    const VkAttachmentReference2KHR* pDepthStencilAttachment;
    uint32_t preserveAttachmentCount;
    const uint32_t* pPreserveAttachments;
} VkSubpassDescription2KHR;

typedef struct VkSubpassDependency2KHR
{
    VkStructureType sType;
    const void* pNext;
    uint32_t srcSubpass;
    uint32_t dstSubpass;
    VkPipelineStageFlags srcStageMask;
    VkPipelineStageFlags dstStageMask;
    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;
    VkDependencyFlags dependencyFlags;
    int32_t viewOffset;
} VkSubpassDependency2KHR;

typedef struct VkRenderPassCreateInfo2KHR
{
    VkStructureType sType;
    const void* pNext;
    VkRenderPassCreateFlags flags;
    uint32_t attachmentCount;
    const VkAttachmentDescription2KHR* pAttachments;
    uint32_t subpassCount;
    const VkSubpassDescription2KHR* pSubpasses;
    uint32_t dependencyCount;
    const VkSubpassDependency2KHR* pDependencies;
    uint32_t correlatedViewMaskCount;
    const uint32_t* pCorrelatedViewMasks;
} VkRenderPassCreateInfo2KHR;

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definitions not provided prior to 1.3.215
//

#if VK_HEADER_VERSION < 215

#    define VK_KHR_fragment_shader_barycentric 1
#    define VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_SPEC_VERSION 1
#    define VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME "VK_KHR_fragment_shader_barycentric"

#    define VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR VkStructureType(1000203000)
#    define VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_EXT VkStructureType(1000203000)

typedef struct VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR
{
    VkStructureType sType;
    void* pNext;
    VkBool32 fragmentShaderBarycentric;
} VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definitions not provided prior to 1.3.231
//

#if VK_HEADER_VERSION < 231

#    define VK_EXT_mesh_shader 1
#    define VK_EXT_MESH_SHADER_SPEC_VERSION 1
#    define VK_EXT_MESH_SHADER_EXTENSION_NAME "VK_EXT_mesh_shader"

#    define VK_SHADER_STAGE_TASK_BIT_EXT VkShaderStageFlagBits(0x00000040)
#    define VK_SHADER_STAGE_MESH_BIT_EXT VkShaderStageFlagBits(0x00000080)

#    define VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT VkStructureType(1000202000)
#    define VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT VkStructureType(1000202001)

typedef struct VkPhysicalDeviceMeshShaderFeaturesEXT {
    VkStructureType    sType;
    void*              pNext;
    VkBool32           taskShader;
    VkBool32           meshShader;
    VkBool32           multiviewMeshShader;
    VkBool32           primitiveFragmentShadingRateMeshShader;
    VkBool32           meshShaderQueries;
} VkPhysicalDeviceMeshShaderFeaturesEXT;

typedef struct VkPhysicalDeviceMeshShaderPropertiesEXT {
    VkStructureType    sType;
    void*              pNext;
    uint32_t           maxTaskWorkGroupTotalCount;
    uint32_t           maxTaskWorkGroupCount[3];
    uint32_t           maxTaskWorkGroupInvocations;
    uint32_t           maxTaskWorkGroupSize[3];
    uint32_t           maxTaskPayloadSize;
    uint32_t           maxTaskSharedMemorySize;
    uint32_t           maxTaskPayloadAndSharedMemorySize;
    uint32_t           maxMeshWorkGroupTotalCount;
    uint32_t           maxMeshWorkGroupCount[3];
    uint32_t           maxMeshWorkGroupInvocations;
    uint32_t           maxMeshWorkGroupSize[3];
    uint32_t           maxMeshSharedMemorySize;
    uint32_t           maxMeshPayloadAndSharedMemorySize;
    uint32_t           maxMeshOutputMemorySize;
    uint32_t           maxMeshPayloadAndOutputMemorySize;
    uint32_t           maxMeshOutputComponents;
    uint32_t           maxMeshOutputVertices;
    uint32_t           maxMeshOutputPrimitives;
    uint32_t           maxMeshOutputLayers;
    uint32_t           maxMeshMultiviewViewCount;
    uint32_t           meshOutputPerVertexGranularity;
    uint32_t           meshOutputPerPrimitiveGranularity;
    uint32_t           maxPreferredTaskWorkGroupInvocations;
    uint32_t           maxPreferredMeshWorkGroupInvocations;
    VkBool32           prefersLocalInvocationVertexOutput;
    VkBool32           prefersLocalInvocationPrimitiveOutput;
    VkBool32           prefersCompactVertexOutput;
    VkBool32           prefersCompactPrimitiveOutput;
} VkPhysicalDeviceMeshShaderPropertiesEXT;

typedef struct VkDrawMeshTasksIndirectCommandEXT {
    uint32_t    groupCountX;
    uint32_t    groupCountY;
    uint32_t    groupCountZ;
} VkDrawMeshTasksIndirectCommandEXT;

typedef void (VKAPI_PTR *PFN_vkCmdDrawMeshTasksEXT)(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
typedef void (VKAPI_PTR *PFN_vkCmdDrawMeshTasksIndirectEXT)(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
typedef void (VKAPI_PTR *PFN_vkCmdDrawMeshTasksIndirectCountEXT)(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definitions not provided prior to 1.1.97
//
#if VK_HEADER_VERSION < 97

typedef uint64_t VkDeviceAddress;

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definitions not provided prior to 1.1.96
//
#if VK_HEADER_VERSION < 96

typedef uint64_t VkDeviceAddress;
#    define VK_KHR_shader_float_controls 1
#    define VK_KHR_SHADER_FLOAT_CONTROLS_SPEC_VERSION 1
#    define VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME "VK_KHR_shader_float_controls"
typedef struct VkPhysicalDeviceFloatControlsPropertiesKHR
{
    VkStructureType sType;
    void* pNext;
    VkBool32 separateDenormSettings;
    VkBool32 separateRoundingModeSettings;
    VkBool32 shaderSignedZeroInfNanPreserveFloat16;
    VkBool32 shaderSignedZeroInfNanPreserveFloat32;
    VkBool32 shaderSignedZeroInfNanPreserveFloat64;
    VkBool32 shaderDenormPreserveFloat16;
    VkBool32 shaderDenormPreserveFloat32;
    VkBool32 shaderDenormPreserveFloat64;
    VkBool32 shaderDenormFlushToZeroFloat16;
    VkBool32 shaderDenormFlushToZeroFloat32;
    VkBool32 shaderDenormFlushToZeroFloat64;
    VkBool32 shaderRoundingModeRTEFloat16;
    VkBool32 shaderRoundingModeRTEFloat32;
    VkBool32 shaderRoundingModeRTEFloat64;
    VkBool32 shaderRoundingModeRTZFloat16;
    VkBool32 shaderRoundingModeRTZFloat32;
    VkBool32 shaderRoundingModeRTZFloat64;
} VkPhysicalDeviceFloatControlsPropertiesKHR;

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definitions not provided prior to 1.1.106
//
#if VK_HEADER_VERSION < 106

#    define VK_EXT_buffer_device_address 1
#    define VK_EXT_BUFFER_DEVICE_ADDRESS_SPEC_VERSION 2
#    define VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME "VK_EXT_buffer_device_address"
typedef struct VkPhysicalDeviceBufferDeviceAddressFeaturesEXT
{
    VkStructureType sType;
    void* pNext;
    VkBool32 bufferDeviceAddress;
    VkBool32 bufferDeviceAddressCaptureReplay;
    VkBool32 bufferDeviceAddressMultiDevice;
} VkPhysicalDeviceBufferDeviceAddressFeaturesEXT;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definitions not provided prior to 1.1.106
//
#if VK_HEADER_VERSION < 105

#define VK_EXT_host_query_reset 1
#define VK_EXT_HOST_QUERY_RESET_SPEC_VERSION 1
#define VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME "VK_EXT_host_query_reset"
typedef struct VkPhysicalDeviceHostQueryResetFeaturesEXT {
    VkStructureType    sType;
    void*              pNext;
    VkBool32           hostQueryReset;
} VkPhysicalDeviceHostQueryResetFeaturesEXT;

typedef void (VKAPI_PTR *PFN_vkResetQueryPoolEXT)(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);




#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definitions not provided prior to 1.2.131
//
#if VK_HEADER_VERSION < 131

#    define VK_API_VERSION_1_2 VK_MAKE_VERSION(1, 2, 0)

#    define VK_KHR_depth_stencil_resolve 1
#    define VK_KHR_DEPTH_STENCIL_RESOLVE_SPEC_VERSION 1
#    define VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME "VK_KHR_depth_stencil_resolve"

#    define VK_KHR_buffer_device_address 1
#    define VK_KHR_BUFFER_DEVICE_ADDRESS_SPEC_VERSION 1
#    define VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME "VK_KHR_buffer_device_address"

#    define VK_KHR_spirv_1_4 1
#    define VK_KHR_SPIRV_1_4_SPEC_VERSION 1
#    define VK_KHR_SPIRV_1_4_EXTENSION_NAME "VK_KHR_spirv_1_4"

#    define VK_ERROR_UNKNOWN VkResult(-13)

#    define VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT VkBufferUsageFlagBits(0x00020000)

#    define VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO VkStructureType(1000244001)
#    define VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2 VkStructureType(1000109000)
#    define VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2 VkStructureType(1000109001)
#    define VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2 VkStructureType(1000109002)
#    define VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2 VkStructureType(1000109003)
#    define VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2 VkStructureType(1000109004)
#    define VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE VkStructureType(1000199001)
#    define VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES VkStructureType(1000199000)
#    define VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES VkStructureType(1000257000)

#    define VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT VkMemoryAllocateFlagBits(0x00000002)

typedef enum VkResolveModeFlagBits
{
    VK_RESOLVE_MODE_NONE = 0,
    VK_RESOLVE_MODE_SAMPLE_ZERO_BIT = 0x00000001,
    VK_RESOLVE_MODE_AVERAGE_BIT = 0x00000002,
    VK_RESOLVE_MODE_MIN_BIT = 0x00000004,
    VK_RESOLVE_MODE_MAX_BIT = 0x00000008,
    VK_RESOLVE_MODE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} VkResolveModeFlagBits;
typedef VkFlags VkResolveModeFlags;

typedef struct VkAttachmentDescription2
{
    VkStructureType sType;
    const void* pNext;
    VkAttachmentDescriptionFlags flags;
    VkFormat format;
    VkSampleCountFlagBits samples;
    VkAttachmentLoadOp loadOp;
    VkAttachmentStoreOp storeOp;
    VkAttachmentLoadOp stencilLoadOp;
    VkAttachmentStoreOp stencilStoreOp;
    VkImageLayout initialLayout;
    VkImageLayout finalLayout;
} VkAttachmentDescription2;

typedef struct VkAttachmentReference2
{
    VkStructureType sType;
    const void* pNext;
    uint32_t attachment;
    VkImageLayout layout;
    VkImageAspectFlags aspectMask;
} VkAttachmentReference2;

typedef struct VkSubpassDescription2
{
    VkStructureType sType;
    const void* pNext;
    VkSubpassDescriptionFlags flags;
    VkPipelineBindPoint pipelineBindPoint;
    uint32_t viewMask;
    uint32_t inputAttachmentCount;
    const VkAttachmentReference2* pInputAttachments;
    uint32_t colorAttachmentCount;
    const VkAttachmentReference2* pColorAttachments;
    const VkAttachmentReference2* pResolveAttachments;
    const VkAttachmentReference2* pDepthStencilAttachment;
    uint32_t preserveAttachmentCount;
    const uint32_t* pPreserveAttachments;
} VkSubpassDescription2;

typedef struct VkSubpassDependency2
{
    VkStructureType sType;
    const void* pNext;
    uint32_t srcSubpass;
    uint32_t dstSubpass;
    VkPipelineStageFlags srcStageMask;
    VkPipelineStageFlags dstStageMask;
    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;
    VkDependencyFlags dependencyFlags;
    int32_t viewOffset;
} VkSubpassDependency2;

typedef struct VkRenderPassCreateInfo2
{
    VkStructureType sType;
    const void* pNext;
    VkRenderPassCreateFlags flags;
    uint32_t attachmentCount;
    const VkAttachmentDescription2* pAttachments;
    uint32_t subpassCount;
    const VkSubpassDescription2* pSubpasses;
    uint32_t dependencyCount;
    const VkSubpassDependency2* pDependencies;
    uint32_t correlatedViewMaskCount;
    const uint32_t* pCorrelatedViewMasks;
} VkRenderPassCreateInfo2;

typedef struct VkSubpassDescriptionDepthStencilResolve
{
    VkStructureType sType;
    const void* pNext;
    VkResolveModeFlagBits depthResolveMode;
    VkResolveModeFlagBits stencilResolveMode;
    const VkAttachmentReference2* pDepthStencilResolveAttachment;
} VkSubpassDescriptionDepthStencilResolve;

#    if 1
typedef struct VkPhysicalDeviceDepthStencilResolveProperties
{
    VkStructureType sType;
    void* pNext;
    VkResolveModeFlags supportedDepthResolveModes;
    VkResolveModeFlags supportedStencilResolveModes;
    VkBool32 independentResolveNone;
    VkBool32 independentResolve;
} VkPhysicalDeviceDepthStencilResolveProperties;
#    endif

typedef struct VkBufferDeviceAddressInfo
{
    VkStructureType sType;
    const void* pNext;
    VkBuffer buffer;
} VkBufferDeviceAddressInfo;

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definitions not provided prior to 1.2.135
//
#if VK_HEADER_VERSION < 135

#    define VK_SHADER_UNUSED_KHR (~0U)

VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkAccelerationStructureKHR)

typedef VkFlags VkBuildAccelerationStructureFlagsKHR;
typedef VkFlags VkGeometryFlagsKHR;

typedef enum VkGeometryTypeKHR
{ // 135
    VK_GEOMETRY_TYPE_TRIANGLES_KHR = 0,
    VK_GEOMETRY_TYPE_AABBS_KHR = 1,
    VK_GEOMETRY_TYPE_INSTANCES_KHR = 2,
} VkGeometryTypeKHR;

typedef struct VkWriteDescriptorSetAccelerationStructureKHR
{ // 135
    VkStructureType sType;
    const void* pNext;
    uint32_t accelerationStructureCount;
    const VkAccelerationStructureKHR* pAccelerationStructures;
} VkWriteDescriptorSetAccelerationStructureKHR;

typedef enum VkGeometryFlagBitsKHR
{
    VK_GEOMETRY_OPAQUE_BIT_KHR = 0x00000001,
    VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR = 0x00000002,
    VK_GEOMETRY_FLAG_BITS_MAX_ENUM_KHR = 0x7FFFFFFF
} VkGeometryFlagBitsKHR;

typedef enum VkGeometryInstanceFlagBitsKHR
{
    VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR = 0x00000001,
    VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_KHR = 0x00000002,
    VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR = 0x00000004,
    VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR = 0x00000008,
    VK_GEOMETRY_INSTANCE_FLAG_BITS_MAX_ENUM_KHR = 0x7FFFFFFF
} VkGeometryInstanceFlagBitsKHR;

typedef enum VkBuildAccelerationStructureFlagBitsKHR
{
    VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR = 0x00000001,
    VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR = 0x00000002,
    VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR = 0x00000004,
    VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR = 0x00000008,
    VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR = 0x00000010,
    VK_BUILD_ACCELERATION_STRUCTURE_FLAG_BITS_MAX_ENUM_KHR = 0x7FFFFFFF
} VkBuildAccelerationStructureFlagBitsKHR;

typedef enum VkAccelerationStructureTypeKHR
{
    VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR = 0,
    VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR = 1,
    VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR = 2,
    VK_ACCELERATION_STRUCTURE_TYPE_MAX_ENUM_KHR = 0x7FFFFFFF
} VkAccelerationStructureTypeKHR;

typedef enum VkRayTracingShaderGroupTypeKHR
{
    VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR = 0,
    VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR = 1,
    VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR = 2,
    VK_RAY_TRACING_SHADER_GROUP_TYPE_MAX_ENUM_KHR = 0x7FFFFFFF
} VkRayTracingShaderGroupTypeKHR;

#define VK_KHR_shader_non_semantic_info 1
#define VK_KHR_SHADER_NON_SEMANTIC_INFO_SPEC_VERSION 1
#define VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME "VK_KHR_shader_non_semantic_info"

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definitions not provided prior to 1.2.162
//
#if VK_HEADER_VERSION < 162

#    define VK_KHR_ray_tracing_pipeline 1
#    define VK_KHR_RAY_TRACING_PIPELINE_SPEC_VERSION 1
#    define VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME "VK_KHR_ray_tracing_pipeline"

#    define VK_KHR_acceleration_structure 1
#    define VK_KHR_ACCELERATION_STRUCTURE_SPEC_VERSION 11
#    define VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME "VK_KHR_acceleration_structure"
#    define VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME "VK_KHR_deferred_host_operations"

typedef VkFlags VkAccelerationStructureCreateFlagsKHR;

#    define VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR VkDescriptorType(1000150000)
#    define VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR VkStructureType(1000347001)
#    define VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR VkStructureType(1000150004)
#    define VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR VkStructureType(1000150007)
#    define VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR VkStructureType(1000150006)
#    define VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR VkStructureType(1000150015)
#    define VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR VkStructureType(1000150016)
#    define VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR VkStructureType(1000150002)
#    define VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR VkStructureType(1000150020)
#    define VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR VkStructureType(1000150000)
#    define VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR VkStructureType(1000150017)
#    define VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR VkStructureType(1000150005)
#    define VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR VkStructureType(1000150013)
#    define VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR VkStructureType(1000347000)

#    define VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR VkBufferUsageFlagBits(0x00100000)
#    define VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR VkBufferUsageFlagBits(0x00000400)
#    define VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR VkBufferUsageFlagBits(0x00080000)

#    define VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR VkPipelineBindPoint(1000165000)
#    define VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR VkPipelineStageFlagBits(0x02000000)

#    define VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR VkAccessFlagBits(0x00200000)
#    define VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR VkAccessFlagBits(0x00400000)

#    define VK_SHADER_STAGE_RAYGEN_BIT_KHR VkShaderStageFlagBits(0x00000100)
#    define VK_SHADER_STAGE_ANY_HIT_BIT_KHR VkShaderStageFlagBits(0x00000200)
#    define VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR VkShaderStageFlagBits(0x00000400)
#    define VK_SHADER_STAGE_MISS_BIT_KHR VkShaderStageFlagBits(0x00000800)
#    define VK_SHADER_STAGE_INTERSECTION_BIT_KHR VkShaderStageFlagBits(0x00001000)
#    define VK_SHADER_STAGE_CALLABLE_BIT_KHR VkShaderStageFlagBits(0x00002000)

VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDeferredOperationKHR)

typedef enum VkBuildAccelerationStructureModeKHR
{
    VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR = 0,
    VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR = 1,
    VK_BUILD_ACCELERATION_STRUCTURE_MODE_MAX_ENUM_KHR = 0x7FFFFFFF
} VkBuildAccelerationStructureModeKHR;

typedef enum VkAccelerationStructureBuildTypeKHR
{
    VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR = 0,
    VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR = 1,
    VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_OR_DEVICE_KHR = 2,
    VK_ACCELERATION_STRUCTURE_BUILD_TYPE_MAX_ENUM_KHR = 0x7FFFFFFF
} VkAccelerationStructureBuildTypeKHR;

typedef struct VkPhysicalDeviceRayTracingPipelineFeaturesKHR
{
    VkStructureType sType;
    void* pNext;
    VkBool32 rayTracingPipeline;
    VkBool32 rayTracingPipelineShaderGroupHandleCaptureReplay;
    VkBool32 rayTracingPipelineShaderGroupHandleCaptureReplayMixed;
    VkBool32 rayTracingPipelineTraceRaysIndirect;
    VkBool32 rayTraversalPrimitiveCulling;
} VkPhysicalDeviceRayTracingPipelineFeaturesKHR;

typedef struct VkPhysicalDeviceAccelerationStructureFeaturesKHR
{
    VkStructureType sType;
    void* pNext;
    VkBool32 accelerationStructure;
    VkBool32 accelerationStructureCaptureReplay;
    VkBool32 accelerationStructureIndirectBuild;
    VkBool32 accelerationStructureHostCommands;
    VkBool32 descriptorBindingAccelerationStructureUpdateAfterBind;
} VkPhysicalDeviceAccelerationStructureFeaturesKHR;

typedef struct VkPhysicalDeviceRayTracingPipelinePropertiesKHR
{
    VkStructureType sType;
    void* pNext;
    uint32_t shaderGroupHandleSize;
    uint32_t maxRayRecursionDepth;
    uint32_t maxShaderGroupStride;
    uint32_t shaderGroupBaseAlignment;
    uint32_t shaderGroupHandleCaptureReplaySize;
    uint32_t maxRayDispatchInvocationCount;
    uint32_t shaderGroupHandleAlignment;
    uint32_t maxRayHitAttributeSize;
} VkPhysicalDeviceRayTracingPipelinePropertiesKHR;

typedef union VkDeviceOrHostAddressKHR
{
    VkDeviceAddress deviceAddress;
    void* hostAddress;
} VkDeviceOrHostAddressKHR;

typedef union VkDeviceOrHostAddressConstKHR
{
    VkDeviceAddress deviceAddress;
    const void* hostAddress;
} VkDeviceOrHostAddressConstKHR;

typedef struct VkAccelerationStructureBuildRangeInfoKHR
{
    uint32_t primitiveCount;
    uint32_t primitiveOffset;
    uint32_t firstVertex;
    uint32_t transformOffset;
} VkAccelerationStructureBuildRangeInfoKHR;

typedef struct VkAccelerationStructureGeometryTrianglesDataKHR
{
    VkStructureType sType;
    const void* pNext;
    VkFormat vertexFormat;
    VkDeviceOrHostAddressConstKHR vertexData;
    VkDeviceSize vertexStride;
    uint32_t maxVertex;
    VkIndexType indexType;
    VkDeviceOrHostAddressConstKHR indexData;
    VkDeviceOrHostAddressConstKHR transformData;
} VkAccelerationStructureGeometryTrianglesDataKHR;

typedef struct VkAccelerationStructureGeometryAabbsDataKHR
{
    VkStructureType sType;
    const void* pNext;
    VkDeviceOrHostAddressConstKHR data;
    VkDeviceSize stride;
} VkAccelerationStructureGeometryAabbsDataKHR;

typedef struct VkAccelerationStructureGeometryInstancesDataKHR
{
    VkStructureType sType;
    const void* pNext;
    VkBool32 arrayOfPointers;
    VkDeviceOrHostAddressConstKHR data;
} VkAccelerationStructureGeometryInstancesDataKHR;

typedef union VkAccelerationStructureGeometryDataKHR
{
    VkAccelerationStructureGeometryTrianglesDataKHR triangles;
    VkAccelerationStructureGeometryAabbsDataKHR aabbs;
    VkAccelerationStructureGeometryInstancesDataKHR instances;
} VkAccelerationStructureGeometryDataKHR;

typedef struct VkAccelerationStructureGeometryKHR
{
    VkStructureType sType;
    const void* pNext;
    VkGeometryTypeKHR geometryType;
    VkAccelerationStructureGeometryDataKHR geometry;
    VkGeometryFlagsKHR flags;
} VkAccelerationStructureGeometryKHR;

typedef struct VkAccelerationStructureCreateInfoKHR
{
    VkStructureType sType;
    const void* pNext;
    VkAccelerationStructureCreateFlagsKHR createFlags;
    VkBuffer buffer;
    VkDeviceSize offset;
    VkDeviceSize size;
    VkAccelerationStructureTypeKHR type;
    VkDeviceAddress deviceAddress;
} VkAccelerationStructureCreateInfoKHR;

typedef struct VkAccelerationStructureBuildGeometryInfoKHR
{
    VkStructureType sType;
    const void* pNext;
    VkAccelerationStructureTypeKHR type;
    VkBuildAccelerationStructureFlagsKHR flags;
    VkBuildAccelerationStructureModeKHR mode;
    VkAccelerationStructureKHR srcAccelerationStructure;
    VkAccelerationStructureKHR dstAccelerationStructure;
    uint32_t geometryCount;
    const VkAccelerationStructureGeometryKHR* pGeometries;
    const VkAccelerationStructureGeometryKHR* const* ppGeometries;
    VkDeviceOrHostAddressKHR scratchData;
} VkAccelerationStructureBuildGeometryInfoKHR;

typedef struct VkStridedDeviceAddressRegionKHR
{
    VkDeviceAddress deviceAddress;
    VkDeviceSize stride;
    VkDeviceSize size;
} VkStridedDeviceAddressRegionKHR;

typedef struct VkPipelineLibraryCreateInfoKHR
{
    VkStructureType sType;
    const void* pNext;
    uint32_t libraryCount;
    const VkPipeline* pLibraries;
} VkPipelineLibraryCreateInfoKHR;

typedef struct VkRayTracingPipelineInterfaceCreateInfoKHR
{
    VkStructureType sType;
    const void* pNext;
    uint32_t maxPipelineRayPayloadSize;
    uint32_t maxPipelineRayHitAttributeSize;
} VkRayTracingPipelineInterfaceCreateInfoKHR;

typedef struct VkAccelerationStructureDeviceAddressInfoKHR
{
    VkStructureType sType;
    const void* pNext;
    VkAccelerationStructureKHR accelerationStructure;
} VkAccelerationStructureDeviceAddressInfoKHR;

typedef struct VkAccelerationStructureBuildSizesInfoKHR
{
    VkStructureType sType;
    const void* pNext;
    VkDeviceSize accelerationStructureSize;
    VkDeviceSize updateScratchSize;
    VkDeviceSize buildScratchSize;
} VkAccelerationStructureBuildSizesInfoKHR;

typedef struct VkRayTracingShaderGroupCreateInfoKHR
{
    VkStructureType sType;
    const void* pNext;
    VkRayTracingShaderGroupTypeKHR type;
    uint32_t generalShader;
    uint32_t closestHitShader;
    uint32_t anyHitShader;
    uint32_t intersectionShader;
    const void* pShaderGroupCaptureReplayHandle;
} VkRayTracingShaderGroupCreateInfoKHR;

typedef struct VkRayTracingPipelineCreateInfoKHR
{
    VkStructureType sType;
    const void* pNext;
    VkPipelineCreateFlags flags;
    uint32_t stageCount;
    const VkPipelineShaderStageCreateInfo* pStages;
    uint32_t groupCount;
    const VkRayTracingShaderGroupCreateInfoKHR* pGroups;
    uint32_t maxPipelineRayRecursionDepth;
    const VkPipelineLibraryCreateInfoKHR* pLibraryInfo;
    const VkRayTracingPipelineInterfaceCreateInfoKHR* pLibraryInterface;
    const VkPipelineDynamicStateCreateInfo* pDynamicState;
    VkPipelineLayout layout;
    VkPipeline basePipelineHandle;
    int32_t basePipelineIndex;
} VkRayTracingPipelineCreateInfoKHR;

typedef VkResult(VKAPI_PTR* PFN_vkCreateAccelerationStructureKHR)(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureKHR* pAccelerationStructure);
typedef void(VKAPI_PTR* PFN_vkDestroyAccelerationStructureKHR)(VkDevice device, VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks* pAllocator);

typedef void(VKAPI_PTR* PFN_vkCmdBuildAccelerationStructuresKHR)(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos);

typedef VkDeviceAddress(VKAPI_PTR* PFN_vkGetAccelerationStructureDeviceAddressKHR)(VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR* pInfo);

typedef void(VKAPI_PTR* PFN_vkGetAccelerationStructureBuildSizesKHR)(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo, const uint32_t* pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo);

typedef VkResult(VKAPI_PTR* PFN_vkCreateRayTracingPipelinesKHR)(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);

typedef VkResult(VKAPI_PTR* PFN_vkGetRayTracingShaderGroupHandlesKHR)(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData);

typedef void(VKAPI_PTR* PFN_vkCmdTraceRaysKHR)(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth);

#endif

//
// Provide *_Compatibility function definitions to workaround different function definitions across different vulkan_core.h versions.
//
typedef VkResult(VKAPI_PTR* PFN_vkCreateRenderPass2KHR_Compatibility)(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass);
typedef VkDeviceAddress(VKAPI_PTR* PFN_vkGetBufferDeviceAddressKHR_Compatibility)(VkDevice device, const VkBufferDeviceAddressInfo* pInfo);

//
//  Definitions not provided prior to 1.3.211
//
#if VK_HEADER_VERSION < 204
#define VK_API_VERSION_1_3 VK_MAKE_VERSION(1, 3, 0)
#endif

#if VK_HEADER_VERSION < 211
typedef enum VkInstanceCreateFlagBits {
    VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR = 0x00000001,
    VK_INSTANCE_CREATE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} VkInstanceCreateFlagBits;
typedef VkFlags VkInstanceCreateFlags;

#define VK_KHR_portability_enumeration 1
#define VK_KHR_PORTABILITY_ENUMERATION_SPEC_VERSION 1
#define VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME "VK_KHR_portability_enumeration"
#endif

//
// Provide VK_KHR_portability_subset from vulkan_beta.h
//
#ifndef VK_KHR_portability_subset
#define VK_KHR_portability_subset 1
#define VK_KHR_PORTABILITY_SUBSET_SPEC_VERSION 1
#define VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME "VK_KHR_portability_subset"
typedef struct VkPhysicalDevicePortabilitySubsetFeaturesKHR {
    VkStructureType    sType;
    void*              pNext;
    VkBool32           constantAlphaColorBlendFactors;
    VkBool32           events;
    VkBool32           imageViewFormatReinterpretation;
    VkBool32           imageViewFormatSwizzle;
    VkBool32           imageView2DOn3DImage;
    VkBool32           multisampleArrayImage;
    VkBool32           mutableComparisonSamplers;
    VkBool32           pointPolygons;
    VkBool32           samplerMipLodBias;
    VkBool32           separateStencilMaskRef;
    VkBool32           shaderSampleRateInterpolationFunctions;
    VkBool32           tessellationIsolines;
    VkBool32           tessellationPointMode;
    VkBool32           triangleFans;
    VkBool32           vertexAttributeAccessBeyondStride;
} VkPhysicalDevicePortabilitySubsetFeaturesKHR;

typedef struct VkPhysicalDevicePortabilitySubsetPropertiesKHR {
    VkStructureType    sType;
    void*              pNext;
    uint32_t           minVertexInputBindingStrideAlignment;
} VkPhysicalDevicePortabilitySubsetPropertiesKHR;
#endif

#ifndef VK_EXT_debug_utils
#define VK_EXT_debug_utils 1
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDebugUtilsMessengerEXT)
#define VK_EXT_DEBUG_UTILS_SPEC_VERSION   2
#define VK_EXT_DEBUG_UTILS_EXTENSION_NAME "VK_EXT_debug_utils"
typedef VkFlags VkDebugUtilsMessengerCallbackDataFlagsEXT;

typedef enum VkDebugUtilsMessageSeverityFlagBitsEXT {
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT = 0x00000001,
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT = 0x00000010,
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT = 0x00000100,
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT = 0x00001000,
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
} VkDebugUtilsMessageSeverityFlagBitsEXT;

typedef enum VkDebugUtilsMessageTypeFlagBitsEXT {
    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT = 0x00000001,
    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT = 0x00000002,
    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT = 0x00000004,
    VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT = 0x00000008,
    VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
} VkDebugUtilsMessageTypeFlagBitsEXT;
typedef VkFlags VkDebugUtilsMessageTypeFlagsEXT;
typedef VkFlags VkDebugUtilsMessageSeverityFlagsEXT;
typedef VkFlags VkDebugUtilsMessengerCreateFlagsEXT;
typedef struct VkDebugUtilsLabelEXT {
    VkStructureType    sType;
    const void*        pNext;
    const char*        pLabelName;
    float              color[4];
} VkDebugUtilsLabelEXT;

typedef struct VkDebugUtilsObjectNameInfoEXT {
    VkStructureType    sType;
    const void*        pNext;
    VkObjectType       objectType;
    uint64_t           objectHandle;
    const char*        pObjectName;
} VkDebugUtilsObjectNameInfoEXT;

typedef struct VkDebugUtilsMessengerCallbackDataEXT {
    VkStructureType                              sType;
    const void*                                  pNext;
    VkDebugUtilsMessengerCallbackDataFlagsEXT    flags;
    const char*                                  pMessageIdName;
    int32_t                                      messageIdNumber;
    const char*                                  pMessage;
    uint32_t                                     queueLabelCount;
    const VkDebugUtilsLabelEXT*                  pQueueLabels;
    uint32_t                                     cmdBufLabelCount;
    const VkDebugUtilsLabelEXT*                  pCmdBufLabels;
    uint32_t                                     objectCount;
    const VkDebugUtilsObjectNameInfoEXT*         pObjects;
} VkDebugUtilsMessengerCallbackDataEXT;

typedef VkBool32 (VKAPI_PTR *PFN_vkDebugUtilsMessengerCallbackEXT)(
    VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
    void*                                            pUserData);

typedef struct VkDebugUtilsMessengerCreateInfoEXT {
    VkStructureType                         sType;
    const void*                             pNext;
    VkDebugUtilsMessengerCreateFlagsEXT     flags;
    VkDebugUtilsMessageSeverityFlagsEXT     messageSeverity;
    VkDebugUtilsMessageTypeFlagsEXT         messageType;
    PFN_vkDebugUtilsMessengerCallbackEXT    pfnUserCallback;
    void*                                   pUserData;
} VkDebugUtilsMessengerCreateInfoEXT;

typedef struct VkDebugUtilsObjectTagInfoEXT {
    VkStructureType    sType;
    const void*        pNext;
    VkObjectType       objectType;
    uint64_t           objectHandle;
    uint64_t           tagName;
    size_t             tagSize;
    const void*        pTag;
} VkDebugUtilsObjectTagInfoEXT;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definitions not provided prior to 1.1.117
//
#if VK_HEADER_VERSION < 117

#define VK_EXT_line_rasterization 1
#define VK_EXT_LINE_RASTERIZATION_SPEC_VERSION 1
#define VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME "VK_EXT_line_rasterization"

#define VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT VkStructureType(1000259000)
#define VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT VkStructureType(1000259001)

typedef enum VkLineRasterizationModeEXT {
    VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT = 0,
    VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT = 1,
    VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT = 2,
    VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT = 3,
    VK_LINE_RASTERIZATION_MODE_BEGIN_RANGE_EXT = VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT,
    VK_LINE_RASTERIZATION_MODE_END_RANGE_EXT = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT,
    VK_LINE_RASTERIZATION_MODE_RANGE_SIZE_EXT = (VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT - VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT + 1),
    VK_LINE_RASTERIZATION_MODE_MAX_ENUM_EXT = 0x7FFFFFFF
} VkLineRasterizationModeEXT;
typedef struct VkPhysicalDeviceLineRasterizationFeaturesEXT {
    VkStructureType    sType;
    void*              pNext;
    VkBool32           rectangularLines;
    VkBool32           bresenhamLines;
    VkBool32           smoothLines;
    VkBool32           stippledRectangularLines;
    VkBool32           stippledBresenhamLines;
    VkBool32           stippledSmoothLines;
} VkPhysicalDeviceLineRasterizationFeaturesEXT;

typedef struct VkPhysicalDeviceLineRasterizationPropertiesEXT {
    VkStructureType    sType;
    void*              pNext;
    uint32_t           lineSubPixelPrecisionBits;
} VkPhysicalDeviceLineRasterizationPropertiesEXT;

typedef struct VkPipelineRasterizationLineStateCreateInfoEXT {
    VkStructureType               sType;
    const void*                   pNext;
    VkLineRasterizationModeEXT    lineRasterizationMode;
    VkBool32                      stippledLineEnable;
    uint32_t                      lineStippleFactor;
    uint16_t                      lineStipplePattern;
} VkPipelineRasterizationLineStateCreateInfoEXT;

typedef void (VKAPI_PTR *PFN_vkCmdSetLineStippleEXT)(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern);

#ifndef VK_NO_PROTOTYPES
VKAPI_ATTR void VKAPI_CALL vkCmdSetLineStippleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    lineStippleFactor,
    uint16_t                                    lineStipplePattern);
#endif


#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definitions not provided prior to 1.1.92
//
#if VK_HEADER_VERSION < 92

#define VK_EXT_calibrated_timestamps 1
#define VK_EXT_CALIBRATED_TIMESTAMPS_SPEC_VERSION 1
#define VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME "VK_EXT_calibrated_timestamps"

#define VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_EXT VkStructureType(1000184000)

typedef enum VkTimeDomainEXT {
    VK_TIME_DOMAIN_DEVICE_EXT = 0,
    VK_TIME_DOMAIN_CLOCK_MONOTONIC_EXT = 1,
    VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_EXT = 2,
    VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_EXT = 3,
    VK_TIME_DOMAIN_BEGIN_RANGE_EXT = VK_TIME_DOMAIN_DEVICE_EXT,
    VK_TIME_DOMAIN_END_RANGE_EXT = VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_EXT,
    VK_TIME_DOMAIN_RANGE_SIZE_EXT = (VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_EXT - VK_TIME_DOMAIN_DEVICE_EXT + 1),
    VK_TIME_DOMAIN_MAX_ENUM_EXT = 0x7FFFFFFF
} VkTimeDomainEXT;

typedef struct VkCalibratedTimestampInfoEXT {
    VkStructureType    sType;
    const void*        pNext;
    VkTimeDomainEXT    timeDomain;
} VkCalibratedTimestampInfoEXT;


typedef VkResult (VKAPI_PTR *PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT)(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount, VkTimeDomainEXT* pTimeDomains);
typedef VkResult (VKAPI_PTR *PFN_vkGetCalibratedTimestampsEXT)(VkDevice device, uint32_t timestampCount, const VkCalibratedTimestampInfoEXT* pTimestampInfos, uint64_t* pTimestamps, uint64_t* pMaxDeviation);

#ifndef VK_NO_PROTOTYPES
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceCalibrateableTimeDomainsEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainEXT*                            pTimeDomains);

VKAPI_ATTR VkResult VKAPI_CALL vkGetCalibratedTimestampsEXT(
    VkDevice                                    device,
    uint32_t                                    timestampCount,
    const VkCalibratedTimestampInfoEXT*         pTimestampInfos,
    uint64_t*                                   pTimestamps,
    uint64_t*                                   pMaxDeviation);
#endif

#endif


#if VK_HEADER_VERSION < 148

// VK_EXT_extended_dynamic_state is a preprocessor guard. Do not pass it to API calls.
#define VK_EXT_extended_dynamic_state 1
#define VK_EXT_EXTENDED_DYNAMIC_STATE_SPEC_VERSION 1
#define VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME "VK_EXT_extended_dynamic_state"
typedef struct VkPhysicalDeviceExtendedDynamicStateFeaturesEXT {
    VkStructureType    sType;
    void*              pNext;
    VkBool32           extendedDynamicState;
} VkPhysicalDeviceExtendedDynamicStateFeaturesEXT;

typedef void (VKAPI_PTR *PFN_vkCmdSetCullModeEXT)(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode);
typedef void (VKAPI_PTR *PFN_vkCmdSetFrontFaceEXT)(VkCommandBuffer commandBuffer, VkFrontFace frontFace);
typedef void (VKAPI_PTR *PFN_vkCmdSetPrimitiveTopologyEXT)(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology);
typedef void (VKAPI_PTR *PFN_vkCmdSetViewportWithCountEXT)(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports);
typedef void (VKAPI_PTR *PFN_vkCmdSetScissorWithCountEXT)(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors);
typedef void (VKAPI_PTR *PFN_vkCmdBindVertexBuffers2EXT)(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes, const VkDeviceSize* pStrides);
typedef void (VKAPI_PTR *PFN_vkCmdSetDepthTestEnableEXT)(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable);
typedef void (VKAPI_PTR *PFN_vkCmdSetDepthWriteEnableEXT)(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable);
typedef void (VKAPI_PTR *PFN_vkCmdSetDepthCompareOpEXT)(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp);
typedef void (VKAPI_PTR *PFN_vkCmdSetDepthBoundsTestEnableEXT)(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable);
typedef void (VKAPI_PTR *PFN_vkCmdSetStencilTestEnableEXT)(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable);
typedef void (VKAPI_PTR *PFN_vkCmdSetStencilOpEXT)(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp);

#ifndef VK_NO_PROTOTYPES
VKAPI_ATTR void VKAPI_CALL vkCmdSetCullModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkCullModeFlags                             cullMode);

VKAPI_ATTR void VKAPI_CALL vkCmdSetFrontFaceEXT(
    VkCommandBuffer                             commandBuffer,
    VkFrontFace                                 frontFace);

VKAPI_ATTR void VKAPI_CALL vkCmdSetPrimitiveTopologyEXT(
    VkCommandBuffer                             commandBuffer,
    VkPrimitiveTopology                         primitiveTopology);

VKAPI_ATTR void VKAPI_CALL vkCmdSetViewportWithCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports);

VKAPI_ATTR void VKAPI_CALL vkCmdSetScissorWithCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors);

VKAPI_ATTR void VKAPI_CALL vkCmdBindVertexBuffers2EXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes,
    const VkDeviceSize*                         pStrides);

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthTestEnable);

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthWriteEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthWriteEnable);

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthCompareOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkCompareOp                                 depthCompareOp);

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBoundsTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBoundsTestEnable);

VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    stencilTestEnable);

VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    VkStencilOp                                 failOp,
    VkStencilOp                                 passOp,
    VkStencilOp                                 depthFailOp,
    VkCompareOp                                 compareOp);
#endif

#endif
