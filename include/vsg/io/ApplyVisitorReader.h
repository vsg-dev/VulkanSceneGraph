#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2025 Chris Djali

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/ReaderWriter.h>

namespace vsg
{
    /// Class to wrap a ReaderWriter and apply a Visitor to everything it loads
    class VSG_DECLSPEC ApplyVisitorReader : public Inherit<ReaderWriter, ApplyVisitorReader>
    {
    public:
        ApplyVisitorReader(vsg::ref_ptr<ReaderWriter> in_child, vsg::ref_ptr<Visitor> in_visitor);
        ApplyVisitorReader(const ApplyVisitorReader& rhs, const CopyOp& copyop = {});

        vsg::ref_ptr<ReaderWriter> child;
        vsg::ref_ptr<Visitor> visitor;

        void read(Input& input) override;
        void write(Output& output) const override;

        vsg::ref_ptr<vsg::Object> read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options = {}) const override;
        vsg::ref_ptr<vsg::Object> read(std::istream& fin, vsg::ref_ptr<const vsg::Options> options = {}) const override;
        vsg::ref_ptr<vsg::Object> read(const uint8_t* ptr, size_t size, vsg::ref_ptr<const vsg::Options> options = {}) const override;

        bool write(const vsg::Object* object, const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options = {}) const override;
        bool write(const vsg::Object* object, std::ostream& fout, vsg::ref_ptr<const vsg::Options> options = {}) const override;

        bool readOptions(vsg::Options& options, vsg::CommandLine& arguments) const override;

        bool getFeatures(Features& features) const override;

    protected:
        mutable std::mutex _visitorMutex;
    };
    VSG_type_name(vsg::ApplyVisitorReader);

} // namespace vsg
