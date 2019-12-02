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
#include <vsg/core/Export.h>
#include <vsg/core/External.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/Object.h>
#include <vsg/core/Objects.h>
#include <vsg/core/Result.h>
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
#include <vsg/nodes/Commands.h>
#include <vsg/nodes/CullGroup.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/VertexIndexDraw.h>

// Traversal header files
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/traversals/ComputeBounds.h>
#include <vsg/traversals/CullTraversal.h>
#include <vsg/traversals/DispatchTraversal.h>

// Threading header files
#include <vsg/threading/OperationQueue.h>
#include <vsg/threading/OperationThreads.h>
#include <vsg/threading/atomics.h>

// User Interface abstraction header files
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/PrintEvents.h>
#include <vsg/ui/TouchEvent.h>
#include <vsg/ui/UIEvent.h>
#include <vsg/ui/WindowEvent.h>

// Viewer header files
#include <vsg/viewer/Camera.h>
#include <vsg/viewer/CloseHandler.h>
#include <vsg/viewer/CopyImageViewToWindow.h>
#include <vsg/viewer/CommandGraph.h>
#include <vsg/viewer/GraphicsStage.h>
#include <vsg/viewer/Presentation.h>
#include <vsg/viewer/ProjectionMatrix.h>
#include <vsg/viewer/RecordAndSubmitTask.h>
#include <vsg/viewer/RenderGraph.h>
#include <vsg/viewer/Trackball.h>
#include <vsg/viewer/View.h>
#include <vsg/viewer/ViewMatrix.h>
#include <vsg/viewer/Viewer.h>
#include <vsg/viewer/Window.h>

// Vulkan related header files
#include <vsg/vk/AllocationCallbacks.h>
#include <vsg/vk/BindIndexBuffer.h>
#include <vsg/vk/BindVertexBuffers.h>
#include <vsg/vk/Buffer.h>
#include <vsg/vk/BufferData.h>
#include <vsg/vk/BufferView.h>
#include <vsg/vk/Command.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/CommandPool.h>
#include <vsg/vk/ComputePipeline.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/CopyImage.h>
#include <vsg/vk/Descriptor.h>
#include <vsg/vk/DescriptorBuffer.h>
#include <vsg/vk/DescriptorImage.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/DescriptorSet.h>
#include <vsg/vk/DescriptorSetLayout.h>
#include <vsg/vk/DescriptorTexelBufferView.h>
#include <vsg/vk/Device.h>
#include <vsg/vk/DeviceMemory.h>
#include <vsg/vk/Dispatch.h>
#include <vsg/vk/Draw.h>
#include <vsg/vk/Extensions.h>
#include <vsg/vk/Fence.h>
#include <vsg/vk/Framebuffer.h>
#include <vsg/vk/GraphicsPipeline.h>
#include <vsg/vk/Image.h>
#include <vsg/vk/ImageData.h>
#include <vsg/vk/ImageView.h>
#include <vsg/vk/Instance.h>
#include <vsg/vk/MemoryManager.h>
#include <vsg/vk/PhysicalDevice.h>
#include <vsg/vk/PipelineBarrier.h>
#include <vsg/vk/PipelineLayout.h>
#include <vsg/vk/PushConstants.h>
#include <vsg/vk/Queue.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/ResourceHints.h>
#include <vsg/vk/Sampler.h>
#include <vsg/vk/Semaphore.h>
#include <vsg/vk/ShaderModule.h>
#include <vsg/vk/ShaderStage.h>
#include <vsg/vk/Stage.h>
#include <vsg/vk/State.h>
#include <vsg/vk/SubmitCommands.h>
#include <vsg/vk/Surface.h>
#include <vsg/vk/Swapchain.h>

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
#include <vsg/io/ReaderWriter_vsg.h>
#include <vsg/io/read.h>
#include <vsg/io/stream.h>
#include <vsg/io/write.h>

// Utility header files
#include <vsg/utils/CommandLine.h>

// Introspection header files
#include <vsg/introspection/c_interface.h>

// Raytracing header files
#include <vsg/raytracing/AccelerationGeometry.h>
#include <vsg/raytracing/AccelerationStructure.h>
#include <vsg/raytracing/AccelerationStructureBuildTraversal.h>
#include <vsg/raytracing/BottomLevelAccelerationStructure.h>
#include <vsg/raytracing/DescriptorAccelerationStructure.h>
#include <vsg/raytracing/RayTracingPipeline.h>
#include <vsg/raytracing/RayTracingShaderGroup.h>
#include <vsg/raytracing/RayTracingStage.h>
#include <vsg/raytracing/TopLevelAccelerationStructure.h>
#include <vsg/raytracing/TraceRays.h>
