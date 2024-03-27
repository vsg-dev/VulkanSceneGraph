#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/common.h>
#include <vsg/nodes/Node.h>

namespace vsg
{

    class VSG_DECLSPEC ShadowSettings : public Inherit<Object, ShadowSettings>
    {
    public:
        explicit ShadowSettings(uint32_t shadowMaps = 1);
        ShadowSettings(const ShadowSettings& rhs, const CopyOp& copyop = {});

        uint32_t shadowMaps = 1;

    public:
        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return ShadowSettings::create(*this, copyop); }
        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~ShadowSettings() {}
    };
    VSG_type_name(vsg::ShadowSettings);

    class VSG_DECLSPEC HardShadows : public Inherit<ShadowSettings, HardShadows>
    {
    public:
        explicit HardShadows(uint32_t in_shadowMaps = 1);
    };

    class VSG_DECLSPEC SoftShadows : public Inherit<ShadowSettings, SoftShadows>
    {
    public:
        explicit SoftShadows(uint32_t in_shadowMaps = 1, float in_penumbraRadius = 0.05f);

        float penumbraRadius = 0.05f;
    };

    class VSG_DECLSPEC PercentageCloserSoftShadows : public Inherit<ShadowSettings, PercentageCloserSoftShadows>
    {
    public:
        explicit PercentageCloserSoftShadows(uint32_t in_shadowMaps = 1);
    };

} // namespace vsg
