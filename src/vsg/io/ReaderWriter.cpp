/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/ReaderWriter.h>
#include <vsg/io/BinaryInput.h>
#include <vsg/io/BinaryOutput.h>
#include <vsg/io/AsciiInput.h>
#include <vsg/io/AsciiOutput.h>

using namespace vsg;

void CompositeReaderWriter::add(ref_ptr<ReaderWriter> reader)
{
    _readerWriters.emplace_back(reader);
}

vsg::ref_ptr<vsg::Object> CompositeReaderWriter::readFile(const vsg::Path& filename) const
{
    for(auto& reader : _readerWriters)
    {
        if (auto object = reader->readFile(filename); object.valid()) return object;
    }
    return vsg::ref_ptr<vsg::Object>();
}

bool CompositeReaderWriter::writeFile(const vsg::Object* object, const vsg::Path& filename) const
{
    for(auto& writer : _readerWriters)
    {
        if (writer->writeFile(object, filename)) return true;
    }
    return false;
}

vsg::ref_ptr<vsg::Object> vsgReaderWriter::readFile(const vsg::Path& filename) const
{
    if (vsg::fileExists(filename))
    {
        auto ext = vsg::fileExtension(filename);
        if (ext=="vsga")
        {
            std::ifstream fin(filename);
            vsg::AsciiInput input(fin);
            return input.readObject("Root");
        }
        else if (ext=="vsgb")
        {
            std::ifstream fin(filename, std::ios::in | std::ios::binary);
            vsg::BinaryInput input(fin);
            return input.readObject("Root");
        }
        else
        {
            return vsg::ref_ptr<vsg::Object>();
        }
    }
    else
    {
        return vsg::ref_ptr<vsg::Object>();
    }
}

bool vsgReaderWriter::writeFile(const vsg::Object* object, const vsg::Path& filename) const
{
    auto ext = vsg::fileExtension(filename);
    if (ext=="vsga")
    {
        std::ofstream fout(filename);
        vsg::AsciiOutput output(fout);
        output.writeObject("Root", object);
        return true;
    }
    else if (ext=="vsgb")
    {
        std::ofstream fout(filename, std::ios::out | std::ios::binary);
        vsg::BinaryOutput output(fout);
        output.writeObject("Root", object);
        return true;
    }
    else
    {
        return false;
    }
}
