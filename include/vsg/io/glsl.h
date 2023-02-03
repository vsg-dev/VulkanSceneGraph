#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2021 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/ReaderWriter.h>

#include <map>

namespace vsg
{

    /// glsl ReaderWriter supports reading and writing GLSL shader files.
    class VSG_DECLSPEC glsl : public vsg::Inherit<vsg::ReaderWriter, glsl>
    {
    public:
        glsl();

        vsg::ref_ptr<vsg::Object> read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options = {}) const override;
        ref_ptr<vsg::Object> read(std::istream& fin, ref_ptr<const Options> options = {}) const override;
        ref_ptr<vsg::Object> read(const uint8_t* ptr, size_t size, ref_ptr<const Options> = {}) const override;

        bool write(const vsg::Object* object, const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options = {}) const override;

        bool getFeatures(Features& features) const override;

        /// return true if .ext is supported
        static bool extensionSupported(const vsg::Path& path);

    protected:
        ref_ptr<Object> createShader(const Path& found_filename, std::string& source, VkShaderStageFlagBits stageFlagBits, vsg::ref_ptr<const vsg::Options> options) const;
    };
    VSG_type_name(vsg::glsl);

} // namespace vsg
