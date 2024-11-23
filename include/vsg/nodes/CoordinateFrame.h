#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Transform.h>

namespace vsg
{

    /// CoordinateFrame provides support for astronomically large coordinates
    class VSG_DECLSPEC CoordinateFrame : public Inherit<Transform, CoordinateFrame>
    {
    public:
        CoordinateFrame();
        CoordinateFrame(const CoordinateFrame& rhs, const CopyOp& copyop = {});

        std::string name;
        dvec3 origin;
        dquat rotation;

        dmat4 transform(const dmat4& mv) const override;

    public:
        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return CoordinateFrame::create(*this, copyop); }
        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
    };
    VSG_type_name(vsg::CoordinateFrame);

} // namespace vsg
