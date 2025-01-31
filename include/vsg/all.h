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
#include <vsg/core/IntrusiveAllocator.h>
#include <vsg/core/Mask.h>
#include <vsg/core/MemorySlots.h>
#include <vsg/core/Object.h>
#include <vsg/core/Objects.h>
#include <vsg/core/ScratchMemory.h>
#include <vsg/core/Value.h>
#include <vsg/core/Version.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/compare.h>
#include <vsg/core/contains.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/type_name.h>
#include <vsg/core/visit.h>

// Maths header files
#include <vsg/maths/box.h>
#include <vsg/maths/clamp.h>
#include <vsg/maths/color.h>
#include <vsg/maths/common.h>
#include <vsg/maths/mat3.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/plane.h>
#include <vsg/maths/quat.h>
#include <vsg/maths/sample.h>
#include <vsg/maths/sphere.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>

// Node header files
#include <vsg/nodes/AbsoluteTransform.h>
#include <vsg/nodes/Bin.h>
#include <vsg/nodes/Compilable.h>
#include <vsg/nodes/CoordinateFrame.h>
#include <vsg/nodes/CullGroup.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/DepthSorted.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/InstrumentationNode.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/Layer.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/RegionOfInterest.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/Switch.h>
#include <vsg/nodes/TileDatabase.h>
#include <vsg/nodes/Transform.h>
#include <vsg/nodes/VertexDraw.h>
#include <vsg/nodes/VertexIndexDraw.h>

// Animation header files
#include <vsg/animation/Animation.h>
#include <vsg/animation/AnimationGroup.h>
#include <vsg/animation/AnimationManager.h>
#include <vsg/animation/CameraAnimationHandler.h>
#include <vsg/animation/CameraSampler.h>
#include <vsg/animation/FindAnimations.h>
#include <vsg/animation/Joint.h>
#include <vsg/animation/JointSampler.h>
#include <vsg/animation/MorphSampler.h>
#include <vsg/animation/TransformSampler.h>
#include <vsg/animation/time_value.h>

// Lighting header files
#include <vsg/lighting/AmbientLight.h>
#include <vsg/lighting/DirectionalLight.h>
#include <vsg/lighting/HardShadows.h>
#include <vsg/lighting/Light.h>
#include <vsg/lighting/PercentageCloserSoftShadows.h>
#include <vsg/lighting/PointLight.h>
#include <vsg/lighting/ShadowSettings.h>
#include <vsg/lighting/SoftShadows.h>
#include <vsg/lighting/SpotLight.h>

// Commands header files
#include <vsg/commands/BeginQuery.h>
#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/commands/BlitImage.h>
#include <vsg/commands/ClearAttachments.h>
#include <vsg/commands/ClearImage.h>
#include <vsg/commands/Command.h>
#include <vsg/commands/Commands.h>
#include <vsg/commands/CopyAndReleaseBuffer.h>
#include <vsg/commands/CopyAndReleaseImage.h>
#include <vsg/commands/CopyImage.h>
#include <vsg/commands/CopyImageToBuffer.h>
#include <vsg/commands/CopyImageViewToWindow.h>
#include <vsg/commands/CopyQueryPoolResults.h>
#include <vsg/commands/Dispatch.h>
#include <vsg/commands/Draw.h>
#include <vsg/commands/DrawIndexed.h>
#include <vsg/commands/DrawIndexedIndirect.h>
#include <vsg/commands/DrawIndirect.h>
#include <vsg/commands/DrawIndirectCommand.h>
#include <vsg/commands/EndQuery.h>
#include <vsg/commands/Event.h>
#include <vsg/commands/ExecuteCommands.h>
#include <vsg/commands/NextSubPass.h>
#include <vsg/commands/PipelineBarrier.h>
#include <vsg/commands/ResetQueryPool.h>
#include <vsg/commands/ResolveImage.h>
#include <vsg/commands/SetDepthBias.h>
#include <vsg/commands/SetLineWidth.h>
#include <vsg/commands/SetPrimitiveTopology.h>
#include <vsg/commands/SetScissor.h>
#include <vsg/commands/SetViewport.h>
#include <vsg/commands/WriteTimestamp.h>

// State header files
#include <vsg/state/ArrayState.h>
#include <vsg/state/BindDescriptorSet.h>
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
#include <vsg/state/PushConstants.h>
#include <vsg/state/QueryPool.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/ResourceHints.h>
#include <vsg/state/Sampler.h>
#include <vsg/state/ShaderModule.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/state/StateCommand.h>
#include <vsg/state/StateSwitch.h>
#include <vsg/state/TessellationState.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/state/ViewDependentState.h>
#include <vsg/state/ViewportState.h>
#include <vsg/state/material.h>

// Threading header files
#include <vsg/threading/ActivityStatus.h>
#include <vsg/threading/Affinity.h>
#include <vsg/threading/Barrier.h>
#include <vsg/threading/DeleteQueue.h>
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
#include <vsg/ui/Keyboard.h>
#include <vsg/ui/PlayEvents.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/PrintEvents.h>
#include <vsg/ui/RecordEvents.h>
#include <vsg/ui/ScrollWheelEvent.h>
#include <vsg/ui/ShiftEventTime.h>
#include <vsg/ui/TouchEvent.h>
#include <vsg/ui/UIEvent.h>
#include <vsg/ui/WindowEvent.h>

// Application header files
#include <vsg/app/Camera.h>
#include <vsg/app/CloseHandler.h>
#include <vsg/app/CommandGraph.h>
#include <vsg/app/CompileManager.h>
#include <vsg/app/CompileTraversal.h>
#include <vsg/app/EllipsoidModel.h>
#include <vsg/app/Presentation.h>
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/RecordAndSubmitTask.h>
#include <vsg/app/RecordTraversal.h>
#include <vsg/app/RenderGraph.h>
#include <vsg/app/SecondaryCommandGraph.h>
#include <vsg/app/Trackball.h>
#include <vsg/app/TransferTask.h>
#include <vsg/app/UpdateOperations.h>
#include <vsg/app/View.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/app/Viewer.h>
#include <vsg/app/Window.h>
#include <vsg/app/WindowAdapter.h>
#include <vsg/app/WindowResizeHandler.h>
#include <vsg/app/WindowTraits.h>

// Vulkan related header files
#include <vsg/vk/AllocationCallbacks.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/CommandPool.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/DescriptorPools.h>
#include <vsg/vk/Device.h>
#include <vsg/vk/DeviceExtensions.h>
#include <vsg/vk/DeviceFeatures.h>
#include <vsg/vk/DeviceMemory.h>
#include <vsg/vk/Fence.h>
#include <vsg/vk/Framebuffer.h>
#include <vsg/vk/Instance.h>
#include <vsg/vk/InstanceExtensions.h>
#include <vsg/vk/MemoryBufferPools.h>
#include <vsg/vk/PhysicalDevice.h>
#include <vsg/vk/Queue.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/ResourceRequirements.h>
#include <vsg/vk/Semaphore.h>
#include <vsg/vk/State.h>
#include <vsg/vk/SubmitCommands.h>
#include <vsg/vk/Surface.h>
#include <vsg/vk/Swapchain.h>
#include <vsg/vk/vk_buffer.h>
#include <vsg/vk/vulkan.h>

// Input/Output header files
#include <vsg/io/AsciiInput.h>
#include <vsg/io/AsciiOutput.h>
#include <vsg/io/BinaryInput.h>
#include <vsg/io/BinaryOutput.h>
#include <vsg/io/DatabasePager.h>
#include <vsg/io/FileSystem.h>
#include <vsg/io/Input.h>
#include <vsg/io/Logger.h>
#include <vsg/io/ObjectFactory.h>
#include <vsg/io/Options.h>
#include <vsg/io/Output.h>
#include <vsg/io/Path.h>
#include <vsg/io/ReaderWriter.h>
#include <vsg/io/VSG.h>
#include <vsg/io/convert_utf.h>
#include <vsg/io/glsl.h>
#include <vsg/io/mem_stream.h>
#include <vsg/io/read.h>
#include <vsg/io/read_line.h>
#include <vsg/io/spirv.h>
#include <vsg/io/stream.h>
#include <vsg/io/tile.h>
#include <vsg/io/txt.h>
#include <vsg/io/write.h>

// Utility header files
#include <vsg/utils/Builder.h>
#include <vsg/utils/CommandLine.h>
#include <vsg/utils/ComputeBounds.h>
#include <vsg/utils/CoordinateSpace.h>
#include <vsg/utils/FindDynamicObjects.h>
#include <vsg/utils/GpuAnnotation.h>
#include <vsg/utils/GraphicsPipelineConfigurator.h>
#include <vsg/utils/Instrumentation.h>
#include <vsg/utils/Intersector.h>
#include <vsg/utils/LineSegmentIntersector.h>
#include <vsg/utils/LoadPagedLOD.h>
#include <vsg/utils/PolytopeIntersector.h>
#include <vsg/utils/PrimitiveFunctor.h>
#include <vsg/utils/Profiler.h>
#include <vsg/utils/PropagateDynamicObjects.h>
#include <vsg/utils/ShaderCompiler.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/utils/SharedObjects.h>

// Text header files
#include <vsg/text/CpuLayoutTechnique.h>
#include <vsg/text/Font.h>
#include <vsg/text/GlyphMetrics.h>
#include <vsg/text/GpuLayoutTechnique.h>
#include <vsg/text/StandardLayout.h>
#include <vsg/text/Text.h>
#include <vsg/text/TextGroup.h>
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

// Mesh shader header files
#include <vsg/meshshaders/DrawMeshTasks.h>
#include <vsg/meshshaders/DrawMeshTasksIndirect.h>
#include <vsg/meshshaders/DrawMeshTasksIndirectCount.h>
