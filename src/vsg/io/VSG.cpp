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
#include <vsg/io/VSG.h>

#include <cstring>
#include <iostream>

using namespace vsg;

// use a static handle that is initialized once at start up to avoid multi-threaded issues associated with calling std::locale::classic().
auto s_class_locale = std::locale::classic();

static VsgVersion parseVersion(std::string version_string)
{
    VsgVersion version{0, 0, 0, 0};

    for (auto& c : version_string)
    {
        if (c == '.') c = ' ';
    }

    std::stringstream str(version_string);

    str >> version.major;
    str >> version.minor;
    str >> version.patch;
    str >> version.soversion;

    return version;
}

VSG::VSG()
{
    _objectFactory = ObjectFactory::instance();
}

VSG::FormatInfo VSG::readHeader(std::istream& fin) const
{
    fin.imbue(s_class_locale);

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
        std::cout << "Header token not matched [" << read_token << "]" << std::endl;
        return FormatInfo(NOT_RECOGNIZED, VsgVersion{0, 0, 0, 0});
    }

    std::string version_string;
    std::getline(fin, version_string);

    auto version = parseVersion(version_string);

    return FormatInfo(type, version);
}

void VSG::writeHeader(std::ostream& fout, const FormatInfo& formatInfo) const
{
    if (formatInfo.first == NOT_RECOGNIZED) return;

    fout.imbue(s_class_locale);
    if (formatInfo.first == BINARY)
        fout << "#vsgb";
    else
        fout << "#vsga";

    auto version = formatInfo.second;
    fout << " " << version.major << "." << version.minor << "." << version.patch << "\n";
}

vsg::ref_ptr<vsg::Object> VSG::read(const vsg::Path& filename, ref_ptr<const Options> options) const
{
    auto ext = vsg::fileExtension(filename);
    if (ext == "vsga" || ext == "vsgt" || ext == "vsgb")
    {
        vsg::Path filenameToUse = findFile(filename, options);
        if (filenameToUse.empty()) return {};

        std::ifstream fin(filenameToUse, std::ios::in | std::ios::binary);
        if (!fin) return {};

        auto [type, version] = readHeader(fin);
        if (type == BINARY)
        {
            vsg::BinaryInput input(fin, _objectFactory, options);
            input.filename = filenameToUse;
            input.version = version;
            return input.readObject("Root");
        }
        else if (type == ASCII)
        {
            vsg::AsciiInput input(fin, _objectFactory, options);
            input.filename = filenameToUse;
            input.version = version;
            return input.readObject("Root");
        }
    }

    // return null as no means for loading file has been found
    return {};
}

vsg::ref_ptr<vsg::Object> VSG::read(std::istream& fin, vsg::ref_ptr<const vsg::Options> options) const
{
    if (options)
    {
        if (!options->extensionHint.empty())
        {
            if (options->extensionHint != "vsgb" && options->extensionHint != "vsgt")
            {
                return {};
            }
        }
    }

    auto [type, version] = readHeader(fin);
    if (type == BINARY)
    {
        vsg::BinaryInput input(fin, _objectFactory, options);
        input.version = version;
        return input.readObject("Root");
    }
    else if (type == ASCII)
    {
        vsg::AsciiInput input(fin, _objectFactory, options);
        input.version = version;
        return input.readObject("Root");
    }

    return {};
}

vsg::ref_ptr<vsg::Object> VSG::read(const uint8_t* ptr, size_t size, vsg::ref_ptr<const vsg::Options> options) const
{
    if (options)
    {
        if (!options->extensionHint.empty())
        {
            if (options->extensionHint != "vsgb" && options->extensionHint != "vsgt")
            {
                return {};
            }
        }
    }

    std::string str(reinterpret_cast<const char*>(ptr), size);
    std::istringstream stream(str);

    auto [type, version] = readHeader(stream);
    if (type == BINARY)
    {
        vsg::BinaryInput input(stream, _objectFactory, options);
        input.version = version;
        return input.readObject("Root");
    }
    else if (type == ASCII)
    {
        vsg::AsciiInput input(stream, _objectFactory, options);
        input.version = version;
        return input.readObject("Root");
    }

    return {};
}

bool VSG::write(const vsg::Object* object, const vsg::Path& filename, ref_ptr<const Options> options) const
{
    auto version = vsgGetVersion();

    if (options)
    {
        std::string version_string;
        if (options->getValue("version", version_string))
        {
            version = parseVersion(version_string);
        }
    }

    auto ext = vsg::fileExtension(filename);
    if (ext == "vsgb")
    {
        std::ofstream fout(filename, std::ios::out | std::ios::binary);
        writeHeader(fout, FormatInfo{BINARY, version});

        vsg::BinaryOutput output(fout, options);
        output.version = version;
        output.writeObject("Root", object);
        return true;
    }
    else if (ext == "vsga" || ext == "vsgt")
    {
        std::ofstream fout(filename);
        writeHeader(fout, FormatInfo{ASCII, version});

        vsg::AsciiOutput output(fout, options);
        output.version = version;
        output.writeObject("Root", object);
        return true;
    }
    else
    {
        return false;
    }
}

bool VSG::write(const vsg::Object* object, std::ostream& fout, ref_ptr<const Options> options) const
{
    auto version = vsgGetVersion();
    bool asciiFormat = true;

    if (options)
    {
        if (!options->extensionHint.empty())
        {
            if (options->extensionHint == "vsgb")
            {
                asciiFormat = false;
            }
            else if (options->extensionHint != "vsgt")
            {
                return false;
            }
        }

        std::string version_string;
        if (options->getValue("version", version_string))
        {
            version = parseVersion(version_string);
        }
    }

    if (asciiFormat)
    {
        writeHeader(fout, FormatInfo(ASCII, version));

        vsg::AsciiOutput output(fout, options);
        output.version = version;
        output.writeObject("Root", object);
        return true;
    }
    else
    {
        writeHeader(fout, FormatInfo(BINARY, version));

        vsg::BinaryOutput output(fout, options);
        output.version = version;
        output.writeObject("Root", object);
        return true;
    }
}

bool VSG::getFeatures(Features& features) const
{
    features.extensionFeatureMap["vsgb"] = static_cast<FeatureMask>(static_cast<uint8_t>(FeatureMask::READ_FILENAME) | static_cast<uint8_t>(FeatureMask::READ_ISTREAM) | static_cast<uint8_t>(FeatureMask::READ_MEMORY) | static_cast<uint8_t>(FeatureMask::WRITE_FILENAME) | static_cast<uint8_t>(FeatureMask::WRITE_OSTREAM));
    features.extensionFeatureMap["vsgt"] = static_cast<FeatureMask>(static_cast<uint8_t>(FeatureMask::READ_FILENAME) | static_cast<uint8_t>(FeatureMask::READ_ISTREAM) | static_cast<uint8_t>(FeatureMask::READ_MEMORY) | static_cast<uint8_t>(FeatureMask::WRITE_FILENAME) | static_cast<uint8_t>(FeatureMask::WRITE_OSTREAM));
    return true;
}
