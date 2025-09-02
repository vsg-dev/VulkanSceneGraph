#include <vsg/core/Array.h>
#include <vsg/state/material.h>
#include <vsg/text/GlyphMetrics.h>
#include <vsg/commands/DrawIndirectCommand.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/quat.h>
#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>
#include <cstdint>
#include <string>

namespace vsg
{
    template class VSG_DECLSPEC vsg::Array<int8_t>;
    template class VSG_DECLSPEC vsg::Array<uint8_t>;
    template class VSG_DECLSPEC vsg::Array<int16_t>;
    template class VSG_DECLSPEC vsg::Array<uint16_t>;
    template class VSG_DECLSPEC vsg::Array<int32_t>;
    template class VSG_DECLSPEC vsg::Array<uint32_t>;
    template class VSG_DECLSPEC vsg::Array<float>;
    template class VSG_DECLSPEC vsg::Array<double>;
    template class VSG_DECLSPEC vsg::Array<std::string>;
    template class VSG_DECLSPEC vsg::Array<vsg::vec2>;
    template class VSG_DECLSPEC vsg::Array<vsg::vec3>;
    template class VSG_DECLSPEC vsg::Array<vsg::vec4>;
    template class VSG_DECLSPEC vsg::Array<vsg::dvec2>;
    template class VSG_DECLSPEC vsg::Array<vsg::dvec3>;
    template class VSG_DECLSPEC vsg::Array<vsg::dvec4>;
    template class VSG_DECLSPEC vsg::Array<vsg::quat>;
    template class VSG_DECLSPEC vsg::Array<vsg::dquat>;
    template class VSG_DECLSPEC vsg::Array<vsg::bvec2>;
    template class VSG_DECLSPEC vsg::Array<vsg::bvec3>;
    template class VSG_DECLSPEC vsg::Array<vsg::bvec4>;
    template class VSG_DECLSPEC vsg::Array<vsg::ubvec2>;
    template class VSG_DECLSPEC vsg::Array<vsg::ubvec3>;
    template class VSG_DECLSPEC vsg::Array<vsg::ubvec4>;

    template class VSG_DECLSPEC vsg::Array<vsg::GlyphMetrics>;
    template class VSG_DECLSPEC vsg::Array<vsg::PbrMaterial>;
    template class VSG_DECLSPEC vsg::Array<vsg::PhongMaterial>;
    template class VSG_DECLSPEC vsg::Array<vsg::DrawIndirectCommand>;
    template class VSG_DECLSPEC vsg::Array<vsg::t_mat4<float>>;
    template class VSG_DECLSPEC vsg::Array<vsg::t_mat4<double>>;
    template class VSG_DECLSPEC vsg::Array<vsg::t_vec2<int>>;
    template class VSG_DECLSPEC vsg::Array<vsg::t_vec2<unsigned int>>;
    template class VSG_DECLSPEC vsg::Array<vsg::t_vec2<short>>;
    template class VSG_DECLSPEC vsg::Array<vsg::t_vec2<unsigned short>>;
    template class VSG_DECLSPEC vsg::Array<vsg::t_vec3<int>>;
    template class VSG_DECLSPEC vsg::Array<vsg::t_vec3<unsigned int>>;
    template class VSG_DECLSPEC vsg::Array<vsg::t_vec3<short>>;
    template class VSG_DECLSPEC vsg::Array<vsg::t_vec3<unsigned short>>;
    template class VSG_DECLSPEC vsg::Array<vsg::t_vec4<int>>;
    template class VSG_DECLSPEC vsg::Array<vsg::t_vec4<unsigned int>>;
    template class VSG_DECLSPEC vsg::Array<vsg::t_vec4<short>>;
    template class VSG_DECLSPEC vsg::Array<vsg::t_vec4<unsigned short>>;
    template class VSG_DECLSPEC vsg::Array<vsg::block64>;
    template class VSG_DECLSPEC vsg::Array<vsg::block128>;
    template class VSG_DECLSPEC vsg::Array<vsg::material>;
} // namespace vsg