/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Version.h>
#include <vsg/io/AsciiInput.h>
#include <vsg/io/AsciiOutput.h>
#include <vsg/io/BinaryInput.h>
#include <vsg/io/BinaryOutput.h>
#include <vsg/io/ReaderWriter_vsg.h>

#include <cstring>
#include <iostream>

using namespace vsg;

// use a static handle that is initialzaed once at start up to avoid multithreaded issues associated with calling std::locale::classic().
auto s_class_locale = std::locale::classic();

ReaderWriter_vsg::ReaderWriter_vsg()
{
    _objectFactory = ObjectFactory::instance();
}

ReaderWriter_vsg::FormatType ReaderWriter_vsg::readHeader(std::istream& fin) const
{
    fin.imbue(s_class_locale);

    // write header
    const char* match_token_ascii = "#vsga";
    const char* match_token_binary = "#vsgb";
    char read_token[5];
    fin.read(read_token, 5);

    FormatType type = NOT_RECOGNIZED;
    if (std::strncmp(match_token_ascii, read_token, 5) == 0)
        type = ASCII;
    else if (std::strncmp(match_token_binary, read_token, 5) == 0)
        type = BINARY;

    if (type == NOT_RECOGNIZED)
    {
        std::cout << "Header token not matched" << std::endl;
        return type;
    }

    char read_line[1024];
    fin.getline(read_line, sizeof(read_line) - 1);
    //std::cout << "First line [" << read_line << "]" << std::endl;

    return type;
}

void ReaderWriter_vsg::writeHeader(std::ostream& fout, FormatType type) const
{
    if (type == NOT_RECOGNIZED) return;

    fout.imbue(s_class_locale);
    if (type == BINARY)
        fout << "#vsgb";
    else
        fout << "#vsga";

    fout << " " << vsgGetVersion() << "\n";
}

vsg::ref_ptr<vsg::Object> ReaderWriter_vsg::read(const vsg::Path& filename, ref_ptr<const Options> options) const
{
    auto ext = vsg::fileExtension(filename);
    if (ext == "vsga" || ext == "vsgt" || ext == "vsgb")
    {
        vsg::Path filenameToUse = options ? findFile(filename, options) : filename;
        if (filenameToUse.empty()) return {};

        std::ifstream fin(filenameToUse, std::ios::in | std::ios::binary);
        if (!fin) return {};

        FormatType type = readHeader(fin);
        if (type == BINARY)
        {
            vsg::BinaryInput input(fin, _objectFactory, options);
            input.filename = filenameToUse;
            return input.readObject("Root");
        }
        else if (type == ASCII)
        {
            vsg::AsciiInput input(fin, _objectFactory, options);
            input.filename = filenameToUse;
            return input.readObject("Root");
        }
    }

    // return null as no means for loading file has been found
    return {};
}

vsg::ref_ptr<vsg::Object> ReaderWriter_vsg::read(std::istream& fin, vsg::ref_ptr<const vsg::Options> options) const
{
    FormatType type = readHeader(fin);
    if (type == BINARY)
    {
        vsg::BinaryInput input(fin, _objectFactory, options);
        return input.readObject("Root");
    }
    else if (type == ASCII)
    {
        vsg::AsciiInput input(fin, _objectFactory, options);
        return input.readObject("Root");
    }

    return vsg::ref_ptr<vsg::Object>();
}

bool ReaderWriter_vsg::write(const vsg::Object* object, const vsg::Path& filename, ref_ptr<const Options> options) const
{
    auto ext = vsg::fileExtension(filename);
    if (ext == "vsgb")
    {
        std::ofstream fout(filename, std::ios::out | std::ios::binary);
        writeHeader(fout, BINARY);

        vsg::BinaryOutput output(fout, options);
        output.writeObject("Root", object);
        return true;
    }
    else if (ext == "vsga" || ext == "vsgt")
    {
        std::ofstream fout(filename);
        writeHeader(fout, ASCII);

        vsg::AsciiOutput output(fout, options);
        output.writeObject("Root", object);
        return true;
    }
    else
    {
        return false;
    }
}

bool ReaderWriter_vsg::write(const vsg::Object* object, std::ostream& fout, ref_ptr<const Options> options) const
{
#if 0
    if (fout.openmode() & std::ios_base::openmode::binary)
    {
        std::cout<<"Binary outputstream"<<std::endl;
        writeHeader(fout, BINARY);

        vsg::BinaryOutput output(fout, options);
        output.writeObject("Root", object);
        return true;
    }
    else
#endif
    {
        std::cout << "Ascii outputstream" << std::endl;
        writeHeader(fout, ASCII);

        vsg::AsciiOutput output(fout, options);
        output.writeObject("Root", object);
        return true;
    }
}
