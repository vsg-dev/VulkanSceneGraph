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
#include <vsg/io/ObjectCache.h>
#include <vsg/io/ReaderWriter.h>
#include <vsg/io/ReaderWriter_vsg.h>

using namespace vsg;

void CompositeReaderWriter::add(ref_ptr<ReaderWriter> reader)
{
    _readerWriters.emplace_back(reader);
}

vsg::ref_ptr<vsg::Object> CompositeReaderWriter::read(const vsg::Path& filename, ref_ptr<const Options> options) const
{
    for (auto& reader : _readerWriters)
    {
        if (auto object = reader->read(filename, options); object.valid()) return object;
    }
    return vsg::ref_ptr<vsg::Object>();
}

bool CompositeReaderWriter::write(const vsg::Object* object, const vsg::Path& filename, ref_ptr<const Options> options) const
{
    for (auto& writer : _readerWriters)
    {
        if (writer->write(object, filename, options)) return true;
    }
    return false;
}

ref_ptr<Object> vsg::read(const Path& filename, ref_ptr<const Options> options)
{
    ref_ptr<Object> object;
    if (options)
    {
        if (options->objectCache)
        {
            object = options->objectCache->get(filename, options);
            if (object)
            {
                return object;
            }
        }

        if (options->readerWriter)
        {
            object = options->readerWriter->read(filename, options);
        }
    }

    if (!object)
    {
        // fallback to using native ReaderWriter_vsg if extension is compatible
        auto ext = vsg::fileExtension(filename);
        if (ext == "vsga" || ext == "vsgt" || ext == "vsgb")
        {
            ReaderWriter_vsg rw;
            object = rw.read(filename, options);
        }
    }

    if (object && options && options->objectCache)
    {
        // place loaded object into the ObjectCache
        options->objectCache->add(object, filename, options);
    }

    return object;
}

bool vsg::write(ref_ptr<Object> object, const Path& filename, ref_ptr<const Options> options)
{
    bool fileWritten = false;
    if (options)
    {
        // don't write the file if it's already contained in the ObjectCache
        if (options->objectCache && options->objectCache->contains(filename, options)) return true;

        if (options->readerWriter)
        {
            fileWritten = options->readerWriter->write(object, filename, options);
        }
    }

    if (!fileWritten)
    {
        // fallback to using native ReaderWriter_vsg if extension is compatible
        auto ext = vsg::fileExtension(filename);
        if (ext == "vsga" || ext == "vsgt" || ext == "vsgb")
        {
            ReaderWriter_vsg rw;
            fileWritten = rw.write(object, filename, options);
        }
    }

    if (fileWritten && options && options->objectCache)
    {
        options->objectCache->add(object, filename, options);
    }

    return fileWritten;
}
