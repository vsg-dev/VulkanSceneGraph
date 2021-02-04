/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/ObjectCache.h>
#include <vsg/io/Options.h>
#include <vsg/io/ReaderWriter.h>
#include <vsg/threading/OperationThreads.h>
#include <vsg/utils/CommandLine.h>

using namespace vsg;

Options::Options()
{
}

Options::Options(const Options& options) :
    Inherit(),
    //    fileCache(options.fileCache),
    objectCache(options.objectCache),
    readerWriters(options.readerWriters),
    operationThreads(options.operationThreads),
    checkFilenameHint(options.checkFilenameHint),
    paths(options.paths),
    extensionHint(options.extensionHint)
{
}

Options::~Options()
{
}

void Options::add(ref_ptr<ReaderWriter> rw)
{
    if (rw) readerWriters.push_back(rw);
}

void Options::add(const ReaderWriters& rws)
{
    for(auto& rw : rws) add(rw);
}

bool Options::readOptions(CommandLine& arguments)
{
    bool read = false;
    for(auto& readerWriter : readerWriters)
    {
        if (readerWriter->readOptions(*this, arguments)) read = true;
    }
    return read;
}
