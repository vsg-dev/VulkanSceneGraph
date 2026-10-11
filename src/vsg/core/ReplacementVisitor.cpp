/* <editor-fold desc="MIT License">

Copyright(c) 2018-2026 Robert Osfield, Chris Djali

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/all.h>

using namespace vsg;

ReplacementVisitor::ReplacementVisitor()
{
}

std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Object&)
{
    return std::nullopt;
}

std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Objects& value)
{
    return apply(static_cast<Object&>(value));
}

std::optional<ref_ptr<Object>> ReplacementVisitor::apply(External& value)
{
    return apply(static_cast<Object&>(value));
}

std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Data& value)
{
    return apply(static_cast<Object&>(value));
}

std::optional<ref_ptr<Object>> ReplacementVisitor::apply(MipmapLayout& value)
{
    return apply(static_cast<Object&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Values
//
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(stringValue& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(wstringValue& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(boolValue& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(intValue& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(uintValue& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(floatValue& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(doubleValue& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(vec2Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(vec3Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(vec4Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(dvec2Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(dvec3Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(dvec4Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(bvec2Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(bvec3Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(bvec4Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ubvec2Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ubvec3Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ubvec4Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(svec2Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(svec3Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(svec4Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(usvec2Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(usvec3Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(usvec4Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ivec2Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ivec3Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ivec4Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(uivec2Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(uivec3Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(uivec4Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(mat2Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(dmat2Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(mat3Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(dmat3Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(mat4Value& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(dmat4Value& value)
{
    return apply(static_cast<Data&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Arrays
//
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(stringArray& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(byteArray& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ubyteArray& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(shortArray& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ushortArray& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(intArray& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(uintArray& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(floatArray& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(doubleArray& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(vec2Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(vec3Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(vec4Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(dvec2Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(dvec3Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(dvec4Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(bvec2Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(bvec3Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(bvec4Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(svec2Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(svec3Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(svec4Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ivec2Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ivec3Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ivec4Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ubvec2Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ubvec3Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ubvec4Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(usvec2Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(usvec3Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(usvec4Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(uivec2Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(uivec3Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(uivec4Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(mat4Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(dmat4Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(block64Array& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(block128Array& value)
{
    return apply(static_cast<Data&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Array2Ds
//
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(byteArray2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ubyteArray2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(shortArray2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ushortArray2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(intArray2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(uintArray2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(floatArray2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(doubleArray2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(vec2Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(vec3Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(vec4Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(dvec2Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(dvec3Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(dvec4Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(bvec2Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(bvec3Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(bvec4Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(svec2Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(svec3Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(svec4Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ivec2Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ivec3Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ivec4Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ubvec2Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ubvec3Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ubvec4Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(usvec2Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(usvec3Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(usvec4Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(uivec2Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(uivec3Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(uivec4Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(block64Array2D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(block128Array2D& value)
{
    return apply(static_cast<Data&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Array3Ds
//
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(byteArray3D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ubyteArray3D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(shortArray3D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ushortArray3D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(intArray3D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(uintArray3D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(floatArray3D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(doubleArray3D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(vec2Array3D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(vec3Array3D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(vec4Array3D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(dvec2Array3D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(dvec3Array3D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(dvec4Array3D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ubvec2Array3D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ubvec3Array3D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ubvec4Array3D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(block64Array3D& value)
{
    return apply(static_cast<Data&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(block128Array3D& value)
{
    return apply(static_cast<Data&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Nodes
//
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Node& value)
{
    return apply(static_cast<Object&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Compilable& value)
{
    return apply(static_cast<Node&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Commands& value)
{
    return apply(static_cast<Node&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Group& value)
{
    return apply(static_cast<Node&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(QuadGroup& value)
{
    return apply(static_cast<Node&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(LOD& value)
{
    return apply(static_cast<Node&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(PagedLOD& value)
{
    return apply(static_cast<Node&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(StateGroup& value)
{
    return apply(static_cast<Group&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(CullGroup& value)
{
    return apply(static_cast<Group&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(CullNode& value)
{
    return apply(static_cast<Node&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Transform& value)
{
    return apply(static_cast<Group&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(MatrixTransform& value)
{
    return apply(static_cast<Transform&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(CoordinateFrame& value)
{
    return apply(static_cast<Transform&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Geometry& value)
{
    return apply(static_cast<Command&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(VertexDraw& value)
{
    return apply(static_cast<Command&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(VertexIndexDraw& value)
{
    return apply(static_cast<Command&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(DepthSorted& value)
{
    return apply(static_cast<Node&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Layer& value)
{
    return apply(static_cast<Node&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Bin& value)
{
    return apply(static_cast<Node&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Switch& value)
{
    return apply(static_cast<Node&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Light& value)
{
    return apply(static_cast<Node&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(AmbientLight& value)
{
    return apply(static_cast<Light&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(DirectionalLight& value)
{
    return apply(static_cast<Light&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(PointLight& value)
{
    return apply(static_cast<Light&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(SpotLight& value)
{
    return apply(static_cast<Light&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(InstrumentationNode& value)
{
    return apply(static_cast<Node&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(RegionOfInterest& value)
{
    return apply(static_cast<Node&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(InstanceNode& value)
{
    return apply(static_cast<Compilable&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(InstanceDraw& value)
{
    return apply(static_cast<Command&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(InstanceDrawIndexed& value)
{
    return apply(static_cast<Command&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Text Objects
//
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Text& value)
{
    return apply(static_cast<Node&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(TextGroup& value)
{
    return apply(static_cast<Node&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(TextTechnique& value)
{
    return apply(static_cast<Object&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(TextLayout& value)
{
    return apply(static_cast<Object&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Animation Objects/Nodes
//
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Animation& value)
{
    return apply(static_cast<Object&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(AnimationGroup& value)
{
    return apply(static_cast<Group&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(AnimationSampler& sampler)
{
    return apply(static_cast<Object&>(sampler));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(JointSampler& sampler)
{
    return apply(static_cast<AnimationSampler&>(sampler));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(MorphSampler& sampler)
{
    return apply(static_cast<AnimationSampler&>(sampler));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(TransformSampler& sampler)
{
    return apply(static_cast<AnimationSampler&>(sampler));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(CameraSampler& sampler)
{
    return apply(static_cast<AnimationSampler&>(sampler));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Joint& value)
{
    return apply(static_cast<Node&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Vulkan Objects
//
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(BufferInfo& value)
{
    return apply(static_cast<Object&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ImageInfo& value)
{
    return apply(static_cast<Object&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ImageView& value)
{
    return apply(static_cast<Object&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Image& value)
{
    return apply(static_cast<Object&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Command& value)
{
    return apply(static_cast<Compilable&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(StateCommand& value)
{
    return apply(static_cast<Command&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(StateSwitch& value)
{
    return apply(static_cast<StateCommand&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(CommandBuffer& value)
{
    return apply(static_cast<Object&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(RenderPass& value)
{
    return apply(static_cast<Object&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(BindDescriptorSet& value)
{
    return apply(static_cast<StateCommand&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(BindDescriptorSets& value)
{
    return apply(static_cast<StateCommand&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(BindViewDescriptorSets& value)
{
    return apply(static_cast<StateCommand&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Descriptor& value)
{
    return apply(static_cast<Object&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(DescriptorBuffer& value)
{
    return apply(static_cast<Descriptor&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(DescriptorImage& value)
{
    return apply(static_cast<Descriptor&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(DescriptorSet& value)
{
    return apply(static_cast<Object&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(BindVertexBuffers& value)
{
    return apply(static_cast<Command&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(BindIndexBuffer& value)
{
    return apply(static_cast<Command&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(BindComputePipeline& value)
{
    return apply(static_cast<StateCommand&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(BindGraphicsPipeline& value)
{
    return apply(static_cast<StateCommand&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(BindRayTracingPipeline& value)
{
    return apply(static_cast<StateCommand&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(GraphicsPipeline& value)
{
    return apply(static_cast<Object&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ComputePipeline& value)
{
    return apply(static_cast<Object&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(RayTracingPipeline& value)
{
    return apply(static_cast<Object&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(GraphicsPipelineState& value)
{
    return apply(static_cast<StateCommand&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ShaderStage& value)
{
    return apply(static_cast<Object&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(VertexInputState& value)
{
    return apply(static_cast<GraphicsPipelineState&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(InputAssemblyState& value)
{
    return apply(static_cast<GraphicsPipelineState&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(TessellationState& value)
{
    return apply(static_cast<GraphicsPipelineState&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ViewportState& value)
{
    return apply(static_cast<GraphicsPipelineState&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(RasterizationState& value)
{
    return apply(static_cast<GraphicsPipelineState&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(MultisampleState& value)
{
    return apply(static_cast<GraphicsPipelineState&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(DepthStencilState& value)
{
    return apply(static_cast<GraphicsPipelineState&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ColorBlendState& value)
{
    return apply(static_cast<GraphicsPipelineState&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(DynamicState& value)
{
    return apply(static_cast<GraphicsPipelineState&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ResourceHints& value)
{
    return apply(static_cast<Object&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Draw& value)
{
    return apply(static_cast<Command&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(DrawIndexed& value)
{
    return apply(static_cast<Command&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ClearAttachments& value)
{
    return apply(static_cast<Command&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ClearColorImage& value)
{
    return apply(static_cast<Command&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ClearDepthStencilImage& value)
{
    return apply(static_cast<Command&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(QueryPool& value)
{
    return apply(static_cast<Object&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ResetQueryPool& value)
{
    return apply(static_cast<Command&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(BeginQuery& value)
{
    return apply(static_cast<Command&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(EndQuery& value)
{
    return apply(static_cast<Command&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(WriteTimestamp& value)
{
    return apply(static_cast<Command&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(CopyQueryPoolResults& value)
{
    return apply(static_cast<Command&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Mesh shading
//
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(DrawMeshTasks& dmt)
{
    return apply(static_cast<Command&>(dmt));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(DrawMeshTasksIndirect& dmti)
{
    return apply(static_cast<Command&>(dmti));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(DrawMeshTasksIndirectCount& dmtic)
{
    return apply(static_cast<Command&>(dmtic));
}

////////////////////////////////////////////////////////////////////////////////
//
// UI Events
//
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(UIEvent& event)
{
    return apply(static_cast<Object&>(event));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(WindowEvent& event)
{
    return apply(static_cast<UIEvent&>(event));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ExposeWindowEvent& event)
{
    return apply(static_cast<WindowEvent&>(event));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ConfigureWindowEvent& event)
{
    return apply(static_cast<WindowEvent&>(event));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(CloseWindowEvent& event)
{
    return apply(static_cast<WindowEvent&>(event));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(FocusInEvent& event)
{
    return apply(static_cast<WindowEvent&>(event));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(FocusOutEvent& event)
{
    return apply(static_cast<WindowEvent&>(event));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(KeyEvent& event)
{
    return apply(static_cast<WindowEvent&>(event));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(KeyPressEvent& event)
{
    return apply(static_cast<KeyEvent&>(event));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(KeyReleaseEvent& event)
{
    return apply(static_cast<KeyEvent&>(event));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(PointerEvent& event)
{
    return apply(static_cast<WindowEvent&>(event));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ButtonPressEvent& event)
{
    return apply(static_cast<PointerEvent&>(event));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ButtonReleaseEvent& event)
{
    return apply(static_cast<PointerEvent&>(event));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(MoveEvent& event)
{
    return apply(static_cast<PointerEvent&>(event));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(TouchEvent& event)
{
    return apply(static_cast<WindowEvent&>(event));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(TouchDownEvent& event)
{
    return apply(static_cast<TouchEvent&>(event));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(TouchUpEvent& event)
{
    return apply(static_cast<TouchEvent&>(event));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(TouchMoveEvent& event)
{
    return apply(static_cast<TouchEvent&>(event));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ScrollWheelEvent& event)
{
    return apply(static_cast<WindowEvent&>(event));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(TerminateEvent& event)
{
    return apply(static_cast<UIEvent&>(event));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(FrameEvent& event)
{
    return apply(static_cast<UIEvent&>(event));
}

////////////////////////////////////////////////////////////////////////////////
//
// util classes
//
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ShaderCompileSettings& shaderCompileSettings)
{
    return apply(static_cast<Object&>(shaderCompileSettings));
}

////////////////////////////////////////////////////////////////////////////////
//
// Viewer classes
//
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Camera& camera)
{
    return apply(static_cast<Object&>(camera));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(CommandGraph& cg)
{
    return apply(static_cast<Group&>(cg));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(SecondaryCommandGraph& cg)
{
    return apply(static_cast<CommandGraph&>(cg));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(RenderGraph& rg)
{
    return apply(static_cast<Group&>(rg));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(View& view)
{
    return apply(static_cast<Group&>(view));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Viewer& viewer)
{
    return apply(static_cast<Object&>(viewer));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ViewMatrix& value)
{
    return apply(static_cast<Object&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(LookAt& value)
{
    return apply(static_cast<ViewMatrix&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(LookDirection& value)
{
    return apply(static_cast<ViewMatrix&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(RelativeViewMatrix& value)
{
    return apply(static_cast<ViewMatrix&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(TrackingViewMatrix& value)
{
    return apply(static_cast<ViewMatrix&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(ProjectionMatrix& value)
{
    return apply(static_cast<Object&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Perspective& value)
{
    return apply(static_cast<ProjectionMatrix&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(Orthographic& value)
{
    return apply(static_cast<ProjectionMatrix&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(RelativeProjection& value)
{
    return apply(static_cast<ProjectionMatrix&>(value));
}
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(EllipsoidPerspective& value)
{
    return apply(static_cast<ProjectionMatrix&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// General classes
//
std::optional<ref_ptr<Object>> ReplacementVisitor::apply(FrameStamp& fs)
{
    return apply(static_cast<Object&>(fs));
}
