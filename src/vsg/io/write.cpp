/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/VSG.h>
#include <vsg/io/glsl.h>
#include <vsg/io/spirv.h>
#include <vsg/io/write.h>
#include <vsg/utils/SharedObjects.h>

using namespace vsg;

bool vsg::write(ref_ptr<Object> object, const Path& filename, ref_ptr<const Options> options)
{
    bool fileWritten = false;
    if (options)
    {
        // don't write the file if it's already contained in the ObjectCache
        if (options->sharedObjects && options->sharedObjects->contains(filename, options)) return true;

        if (!options->readerWriters.empty())
        {
            for (auto& readerWriter : options->readerWriters)
            {
                fileWritten = readerWriter->write(object, filename, options);
                if (fileWritten) break;
            }
        }
    }

    if (!fileWritten)
    {
        // fallback to using native VSG if extension is compatible
        auto ext = vsg::lowerCaseFileExtension(filename);
        if (ext == ".vsga" || ext == ".vsgt" || ext == ".vsgb")
        {
            VSG rw;
            fileWritten = rw.write(object, filename, options);
        }
        else if (ext == ".spv")
        {
            spirv rw;
            fileWritten = rw.write(object, filename, options);
        }
        else if (glsl::extensionSupported(ext))
        {
            glsl rw;
            fileWritten = rw.write(object, filename, options);
        }
    }

    if (fileWritten && options && options->sharedObjects)
    {
        options->sharedObjects->add(object, filename, options);
    }

    return fileWritten;
}
