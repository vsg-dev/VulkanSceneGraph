#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

// Core header files
#include <vsg/core/Allocator.h>
#include <vsg/core/Array.h>
#include <vsg/core/Array2D.h>
#include <vsg/core/Array3D.h>
#include <vsg/core/Auxiliary.h>
#include <vsg/core/ConstVisitor.h>
#include <vsg/core/Data.h>
#include <vsg/core/Exception.h>
#include <vsg/core/Export.h>
#include <vsg/core/External.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/Object.h>
#include <vsg/core/Objects.h>
#include <vsg/core/ScratchMemory.h>
#include <vsg/core/Value.h>
#include <vsg/core/Version.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/type_name.h>

// Maths header files
#include <vsg/maths/box.h>
#include <vsg/maths/mat3.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/plane.h>
#include <vsg/maths/quat.h>
#include <vsg/maths/sphere.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>

// Node header files
#include <vsg/nodes/Bin.h>
#include <vsg/nodes/CullGroup.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/DepthSorted.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/MaskGroup.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/Switch.h>
#include <vsg/nodes/Transform.h>
#include <vsg/nodes/VertexIndexDraw.h>

// Commands header files
#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/commands/BlitImage.h>
#include <vsg/commands/ClearAttachments.h>
#include <vsg/commands/Command.h>
#include <vsg/commands/Commands.h>
#include <vsg/commands/CopyAndReleaseBuffer.h>
#include <vsg/commands/CopyAndReleaseImage.h>
#include <vsg/commands/CopyImage.h>
#include <vsg/commands/CopyImageToBuffer.h>
#include <vsg/commands/Dispatch.h>
#include <vsg/commands/Draw.h>
#include <vsg/commands/DrawIndexed.h>
#include <vsg/commands/DrawIndexedIndirect.h>
#include <vsg/commands/DrawIndirect.h>
#include <vsg/commands/DrawIndirectCommand.h>
#include <vsg/commands/Event.h>
#include <vsg/commands/NextSubPass.h>
#include <vsg/commands/PipelineBarrier.h>
#include <vsg/commands/PushConstants.h>
#include <vsg/commands/SetDepthBias.h>
#include <vsg/commands/SetLineWidth.h>
#include <vsg/commands/SetScissor.h>
#include <vsg/commands/SetViewport.h>

// State header files
#include <vsg/state/Buffer.h>
#include <vsg/state/BufferInfo.h>
#include <vsg/state/BufferView.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/ComputePipeline.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/Descriptor.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/DescriptorSet.h>
#include <vsg/state/DescriptorSetLayout.h>
#include <vsg/state/DescriptorTexelBufferView.h>
#include <vsg/state/DynamicState.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/Image.h>
#include <vsg/state/ImageInfo.h>
#include <vsg/state/ImageView.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/PipelineLayout.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/ResourceHints.h>
#include <vsg/state/Sampler.h>
#include <vsg/state/ShaderModule.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/state/StateCommand.h>
#include <vsg/state/TessellationState.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/state/ViewportState.h>
#include <vsg/state/material.h>

// Traversal header files
#include <vsg/traversals/ArrayState.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/traversals/ComputeBounds.h>
#include <vsg/traversals/Intersector.h>
#include <vsg/traversals/LineSegmentIntersector.h>
#include <vsg/traversals/LoadPagedLOD.h>
#include <vsg/traversals/RecordTraversal.h>

// Threading header files
#include <vsg/threading/ActivityStatus.h>
#include <vsg/threading/Affinity.h>
#include <vsg/threading/Barrier.h>
#include <vsg/threading/FrameBlock.h>
#include <vsg/threading/Latch.h>
#include <vsg/threading/OperationQueue.h>
#include <vsg/threading/OperationThreads.h>
#include <vsg/threading/atomics.h>

// User Interface abstraction header files
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/ui/CollectEvents.h>
#include <vsg/ui/FrameStamp.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PlayEvents.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/PrintEvents.h>
#include <vsg/ui/RecordEvents.h>
#include <vsg/ui/ScrollWheelEvent.h>
#include <vsg/ui/ShiftEventTime.h>
#include <vsg/ui/TouchEvent.h>
#include <vsg/ui/UIEvent.h>
#include <vsg/ui/WindowEvent.h>

// Viewer header files
#include <vsg/viewer/Camera.h>
#include <vsg/viewer/CloseHandler.h>
#include <vsg/viewer/CommandGraph.h>
#include <vsg/viewer/CopyImageViewToWindow.h>
#include <vsg/viewer/EllipsoidModel.h>
#include <vsg/viewer/ExecuteCommands.h>
#include <vsg/viewer/Presentation.h>
#include <vsg/viewer/ProjectionMatrix.h>
#include <vsg/viewer/RecordAndSubmitTask.h>
#include <vsg/viewer/RenderGraph.h>
#include <vsg/viewer/Trackball.h>
#include <vsg/viewer/View.h>
#include <vsg/viewer/ViewMatrix.h>
#include <vsg/viewer/Viewer.h>
#include <vsg/viewer/Window.h>
#include <vsg/viewer/WindowAdapter.h>
#include <vsg/viewer/WindowResizeHandler.h>
#include <vsg/viewer/WindowTraits.h>

// Vulkan related header files
#include <vsg/vk/AllocationCallbacks.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/CommandPool.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/Device.h>
#include <vsg/vk/DeviceFeatures.h>
#include <vsg/vk/DeviceMemory.h>
#include <vsg/vk/Extensions.h>
#include <vsg/vk/Fence.h>
#include <vsg/vk/Framebuffer.h>
#include <vsg/vk/Instance.h>
#include <vsg/vk/MemoryBufferPools.h>
#include <vsg/vk/PhysicalDevice.h>
#include <vsg/vk/Queue.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/ResourceRequirements.h>
#include <vsg/vk/Semaphore.h>
#include <vsg/vk/ShaderCompiler.h>
#include <vsg/vk/State.h>
#include <vsg/vk/SubmitCommands.h>
#include <vsg/vk/Surface.h>
#include <vsg/vk/Swapchain.h>
#include <vsg/vk/vk_buffer.h>

// Input/Output header files
#include <vsg/io/AsciiInput.h>
#include <vsg/io/AsciiOutput.h>
#include <vsg/io/BinaryInput.h>
#include <vsg/io/BinaryOutput.h>
#include <vsg/io/DatabasePager.h>
#include <vsg/io/FileSystem.h>
#include <vsg/io/Input.h>
#include <vsg/io/ObjectCache.h>
#include <vsg/io/ObjectFactory.h>
#include <vsg/io/Options.h>
#include <vsg/io/Output.h>
#include <vsg/io/ReaderWriter.h>
#include <vsg/io/VSG.h>
#include <vsg/io/read.h>
#include <vsg/io/read_line.h>
#include <vsg/io/spirv.h>
#include <vsg/io/stream.h>
#include <vsg/io/write.h>

// Utility header files
#include <vsg/utils/AnimationPath.h>
#include <vsg/utils/Builder.h>
#include <vsg/utils/CommandLine.h>

// Introspection header files
#include <vsg/introspection/c_interface.h>

// Text header files
#include <vsg/text/CpuLayoutTechnique.h>
#include <vsg/text/Font.h>
#include <vsg/text/GlyphMetrics.h>
#include <vsg/text/GpuLayoutTechnique.h>
#include <vsg/text/StandardLayout.h>
#include <vsg/text/Text.h>
#include <vsg/text/TextLayout.h>
#include <vsg/text/TextTechnique.h>

// Ray tracing header files
#include <vsg/raytracing/AccelerationGeometry.h>
#include <vsg/raytracing/AccelerationStructure.h>
#include <vsg/raytracing/BottomLevelAccelerationStructure.h>
#include <vsg/raytracing/BuildAccelerationStructureTraversal.h>
#include <vsg/raytracing/DescriptorAccelerationStructure.h>
#include <vsg/raytracing/RayTracingPipeline.h>
#include <vsg/raytracing/RayTracingShaderGroup.h>
#include <vsg/raytracing/TopLevelAccelerationStructure.h>
#include <vsg/raytracing/TraceRays.h>

// RTX mesh  header files
#include <vsg/rtx/DrawMeshTasks.h>
#include <vsg/rtx/DrawMeshTasksIndirect.h>
#include <vsg/rtx/DrawMeshTasksIndirectCommand.h>
#include <vsg/rtx/DrawMeshTasksIndirectCount.h>
