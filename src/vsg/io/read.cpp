/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/ObjectCache.h>
#include <vsg/io/ReaderWriter_vsg.h>
#include <vsg/io/read.h>

#include <vsg/threading/OperationProcessor.h>

#include <thread>
#include <iostream>
#include <chrono>
#include <condition_variable>
#include <typeinfo>

using namespace vsg;

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


struct ReadOperation : public Operation
{
    ReadOperation(const Path& f, ref_ptr<const Options> opt, ref_ptr<Object>& obj, ref_ptr<Latch> l) :
        filename(f),
        options(opt),
        object(obj),
        latch(l) {}

    void run() override
    {
        object = vsg::read(filename, options);
        latch->count_down();
    }

    Path filename;
    ref_ptr<const Options> options;
    ref_ptr<Object>& object;
    ref_ptr<Latch> latch;
};


PathObjects vsg::read(const Paths& filenames, ref_ptr<const Options> options)
{
    ref_ptr<OperationProcessor> operationProcessor;
    if (options) operationProcessor = options->operationProcessor;

    auto before_vsg_load = std::chrono::steady_clock::now();

    PathObjects entries;

    if (operationProcessor && filenames.size()>1)
    {
        // set up the entries container for operations to write to.
        for(auto& filename : filenames)
        {
            entries[filename] = nullptr;
        }

        // use latch to syncronize this thread with the file reading threads
        ref_ptr<Latch> latch(new Latch(filenames.size()));

        // add operations
        for(auto& [filename, object] : entries)
        {
            operationProcessor->add(ref_ptr<Operation>(new ReadOperation(filename, options, object, latch)));
        }

        // use this thread to read the files as well
        operationProcessor->run();

        // wait till all the read opeartions have completed
        latch->wait();
    }
    else
    {
        // run reads single threaded
        for(auto& filename : filenames)
        {
            if (!filename.empty())
            {
                entries[filename] = vsg::read(filename, options);
            }
            else
            {
                entries[filename] = nullptr;
            }
        }
    }

    auto vsg_loadTime = std::chrono::duration<double, std::chrono::milliseconds::period>(std::chrono::steady_clock::now() - before_vsg_load).count();
    std::cout<<"After batch load() time =  "<<vsg_loadTime<<std::endl;

    return entries;
}
