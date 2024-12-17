#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/sphere.h>
#include <vsg/nodes/Node.h>

namespace vsg
{

    /// RegionOfInterest node is inform applications/algorithms extents that should take account of.
    class VSG_DECLSPEC RegionOfInterest : public Inherit<Node, RegionOfInterest>
    {
    public:
        RegionOfInterest();
        RegionOfInterest(const RegionOfInterest& rhs, const CopyOp& copyop = {});

        Mask mask = MASK_ALL;
        std::string name;
        std::vector<dvec3> points;

    public:
        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return RegionOfInterest::create(*this, copyop); }
        int compare(const Object& rhs) const override;

        void accept(Visitor& visitor) override
        {
            if ((visitor.traversalMask & (visitor.overrideMask | mask)) != MASK_OFF) visitor.apply(*this);
        }
        void accept(ConstVisitor& visitor) const override
        {
            if ((visitor.traversalMask & (visitor.overrideMask | mask)) != MASK_OFF) visitor.apply(*this);
        }
        void accept(RecordTraversal& visitor) const override
        {
            if ((visitor.traversalMask & (visitor.overrideMask | mask)) != MASK_OFF) visitor.apply(*this);
        }

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~RegionOfInterest();
    };
    VSG_type_name(vsg::RegionOfInterest);

} // namespace vsg
