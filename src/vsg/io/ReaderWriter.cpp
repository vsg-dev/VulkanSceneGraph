/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/ReaderWriter.h>
#include <vsg/utils/CommandLine.h>

using namespace vsg;

void CompositeReaderWriter::add(ref_ptr<ReaderWriter> reader)
{
    readerWriters.emplace_back(reader);
}

void CompositeReaderWriter::read(Input& input)
{
    readerWriters.clear();
    uint32_t count = input.readValue<uint32_t>("NumReaderWriters");
    for (uint32_t i = 0; i < count; ++i)
    {
        auto rw = input.readObject<ReaderWriter>("ReaderWriter");
        if (rw) readerWriters.push_back(rw);
    }
}

void CompositeReaderWriter::write(Output& output) const
{
    output.writeValue<uint32_t>("NumReaderWriters", readerWriters.size());
    for (auto& rw : readerWriters)
    {
        output.writeObject("ReaderWriter", rw);
    }
}

vsg::ref_ptr<vsg::Object> CompositeReaderWriter::read(const vsg::Path& filename, ref_ptr<const Options> options) const
{
    for (auto& reader : readerWriters)
    {
        if (auto object = reader->read(filename, options); object.valid()) return object;
    }
    return vsg::ref_ptr<vsg::Object>();
}

vsg::ref_ptr<vsg::Object> CompositeReaderWriter::read(std::istream& fin, ref_ptr<const Options> options) const
{
    for (auto& reader : readerWriters)
    {
        if (auto object = reader->read(fin, options); object.valid()) return object;
    }
    return vsg::ref_ptr<vsg::Object>();
}

vsg::ref_ptr<vsg::Object> CompositeReaderWriter::read(const uint8_t* ptr, size_t size, vsg::ref_ptr<const vsg::Options> options) const
{
    for (auto& reader : readerWriters)
    {
        if (auto object = reader->read(ptr, size, options); object.valid()) return object;
    }
    return vsg::ref_ptr<vsg::Object>();
}

bool CompositeReaderWriter::write(const vsg::Object* object, const vsg::Path& filename, ref_ptr<const Options> options) const
{
    for (auto& writer : readerWriters)
    {
        if (writer->write(object, filename, options)) return true;
    }
    return false;
}

bool CompositeReaderWriter::write(const vsg::Object* object, std::ostream& fout, vsg::ref_ptr<const vsg::Options> options) const
{
    for (auto& writer : readerWriters)
    {
        if (writer->write(object, fout, options)) return true;
    }
    return false;
}

bool CompositeReaderWriter::readOptions(vsg::Options& options, vsg::CommandLine& arguments) const
{
    bool result = false;
    for (auto& rw : readerWriters)
    {
        if (rw->readOptions(options, arguments)) result = true;
    }
    return result;
}

bool CompositeReaderWriter::getFeatures(Features& features) const
{
    bool result = false;
    for (auto& rw : readerWriters)
    {
        if (rw->getFeatures(features)) result = true;
    }
    return result;
}
