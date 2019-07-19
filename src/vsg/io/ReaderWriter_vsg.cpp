/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/AsciiInput.h>
#include <vsg/io/AsciiOutput.h>
#include <vsg/io/BinaryInput.h>
#include <vsg/io/BinaryOutput.h>
#include <vsg/io/ReaderWriter_vsg.h>

using namespace vsg;

ReaderWriter_vsg::ReaderWriter_vsg()
{
    _objectFactory = new vsg::ObjectFactory;
}

vsg::ref_ptr<vsg::Object> ReaderWriter_vsg::readFile(const vsg::Path& filename, ref_ptr<const Options> options) const
{
    if (vsg::fileExists(filename))
    {
        auto ext = vsg::fileExtension(filename);
        if (ext == "vsga" || ext == "vsgt")
        {
            std::ifstream fin(filename);
            vsg::AsciiInput input(fin, _objectFactory, options);
            return input.readObject("Root");
        }
        else if (ext == "vsgb")
        {
            std::ifstream fin(filename, std::ios::in | std::ios::binary);
            vsg::BinaryInput input(fin, _objectFactory, options);
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

vsg::ref_ptr<vsg::Object> ReaderWriter_vsg::readFile(std::istream& fin, vsg::ref_ptr<const vsg::Options> options) const
{
    Path filename; // TODO need to determine extension using options?
    auto ext = vsg::fileExtension(filename);
    if (ext == "vsga" || ext == "vsgt")
    {
        vsg::AsciiInput input(fin, _objectFactory, options);
        return input.readObject("Root");
    }
    else if (ext == "vsgb")
    {
        vsg::BinaryInput input(fin, _objectFactory, options);
        return input.readObject("Root");
    }
    else
    {
        return vsg::ref_ptr<vsg::Object>();
    }
}

bool ReaderWriter_vsg::writeFile(const vsg::Object* object, const vsg::Path& filename, ref_ptr<const Options> options) const
{
    auto ext = vsg::fileExtension(filename);
    if (ext == "vsga" || ext == "vsgt")
    {
        std::ofstream fout(filename);
        vsg::AsciiOutput output(fout, options);
        output.writeObject("Root", object);
        return true;
    }
    else if (ext == "vsgb")
    {
        std::ofstream fout(filename, std::ios::out | std::ios::binary);
        vsg::BinaryOutput output(fout, options);
        output.writeObject("Root", object);
        return true;
    }
    else
    {
        return false;
    }
}

bool ReaderWriter_vsg::writeFile(const vsg::Object* object, std::ostream& fout, ref_ptr<const Options> options) const
{
    Path filename; // TODO need to determine extension using options?
    auto ext = vsg::fileExtension(filename);
    if (ext == "vsga" || ext == "vsgt")
    {
        vsg::AsciiOutput output(fout, options);
        output.writeObject("Root", object);
        return true;
    }
    else if (ext == "vsgb")
    {
        vsg::BinaryOutput output(fout, options);
        output.writeObject("Root", object);
        return true;
    }
    else
    {
        return false;
    }
}
