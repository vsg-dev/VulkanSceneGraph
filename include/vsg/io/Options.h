#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/io/FileSystem.h>

namespace vsg
{

    //class FileCache;
    class ObjectCache;
    class ReaderWriter;
    class OperationThreads;
    class CommandLine;

    class VSG_DECLSPEC Options : public Inherit<Object, Options>
    {
    public:
        Options();
        Options(ref_ptr<ReaderWriter> rw);
        Options(const Options& options);

        Options& operator=(const Options& rhs) = delete;

        /// read command line options, assign values to this options object to later use wiht reading/writing files
        virtual bool readOptions(CommandLine& arguments);

        //ref_ptr<FileCache> fileCache;
        ref_ptr<ObjectCache> objectCache;
        ref_ptr<ReaderWriter> readerWriter;
        ref_ptr<OperationThreads> operationThreads;
        Paths paths;

    protected:
        virtual ~Options();
    };
    VSG_type_name(vsg::Options);

} // namespace vsg
