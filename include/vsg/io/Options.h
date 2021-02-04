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

    using ReaderWriters = std::vector<ref_ptr<ReaderWriter>>;

    class VSG_DECLSPEC Options : public Inherit<Object, Options>
    {
    public:
        Options();
        explicit Options(const Options& options);

        template<typename... Args>
        explicit Options(Args... args)
        {
            (add(args) , ...);
        }

        Options& operator=(const Options& rhs) = delete;

        /// read command line options, assign values to this options object to later use with reading/writing files
        virtual bool readOptions(CommandLine& arguments);

        void add(ref_ptr<ReaderWriter> rw = {});
        void add(const ReaderWriters& rws);

        //ref_ptr<FileCache> fileCache;
        ref_ptr<ObjectCache> objectCache;
        ReaderWriters readerWriters;
        ref_ptr<OperationThreads> operationThreads;

        /// Hint to use when searching for Paths with vsg::findFile(filename, options);
        enum FindFileHint
        {
            CHECK_ORIGINAL_FILENAME_EXISTS_FIRST, /// check the filename exists with it's original path after failing to find it in the Options::path list.
            CHECK_ORIGINAL_FILENAME_EXISTS_LAST,  /// check the filename exists with it's original path after failing to find it in the Options::path list.
            ONLY_CHECK_PATHS                      /// only check the filename exists in the Options::paths
        };
        FindFileHint checkFilenameHint = CHECK_ORIGINAL_FILENAME_EXISTS_FIRST;

        Paths paths;

        std::string extensionHint;

    protected:
        virtual ~Options();
    };
    VSG_type_name(vsg::Options);

} // namespace vsg
