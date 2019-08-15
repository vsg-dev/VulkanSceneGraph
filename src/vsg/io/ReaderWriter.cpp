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

#include <thread>
#include <iostream>
#include <chrono>

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

struct Active : public Object
{
    Active() : active(true) {}

    std::atomic_bool active;

    explicit operator bool() const noexcept { return active; }

protected:
    virtual ~Active() {}
};

struct Operation : public Object
{
    Operation(const Path& f, ref_ptr<const Options> opt, ref_ptr<Object>& obj, std::atomic_uint& controlVar) :
        filename(f),
        options(opt),
        object(obj),
        readLeftToComplete(controlVar) {}

    Path filename;
    ref_ptr<const Options> options;
    ref_ptr<Object>& object;
    std::atomic_uint& readLeftToComplete;
};

struct OperationQueue : public Object
{
    std::mutex _mutex;
    std::list<ref_ptr<Operation>> _queue;

    void add(ref_ptr<Operation> operation)
    {
        std::lock_guard<std::mutex> guard(_mutex);
        _queue.emplace_back(operation);
    }

    ref_ptr<Operation> take()
    {
        std::lock_guard<std::mutex> guard(_mutex);
        if (_queue.empty()) return {};

        ref_ptr<Operation> operation = _queue.front();

        _queue.erase(_queue.begin());

        return operation;
    }
};


PathObjects vsg::read(const Paths& filenames, ref_ptr<const Options> options)
{
    enum ThreadingModel
    {
        SingleThreaded,
        ThreadPerLoad,
        ThreadPool
    };

    //ThreadingModel threadingModel = SingleThreaded;
    //ThreadingModel threadingModel = ThreadPerLoad;
    ThreadingModel threadingModel = ThreadPool;

// start of thread pool setup
    ref_ptr<OperationQueue> operationQueue;
    ref_ptr<Active> active;
    std::vector<std::thread> poolThreads;

    if (threadingModel == ThreadPool)
    {
        operationQueue = new OperationQueue;
        active = new Active;

        auto run = [operationQueue, active]()
        {
            while(*active)
            {
                ref_ptr<Operation> operation = operationQueue->take();
                if (operation)
                {
                    operation->object = vsg::read(operation->filename, operation->options);
                    --(operation->readLeftToComplete);
                }
            }
        };

        size_t numThreads = 16;
        for(size_t i=0; i<numThreads; ++i)
        {
            poolThreads.emplace_back(std::thread(run));
        }
    }
// end of thread pool setup

    auto before_vsg_load = std::chrono::steady_clock::now();

    PathObjects entries;

    switch(threadingModel)
    {
        case(SingleThreaded) :
        {
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
            break;
        }

        case(ThreadPerLoad) :
        {
            auto readFile = [](const Path& filename, ref_ptr<const Options> opt, ref_ptr<Object>& object)
            {
                object = vsg::read(filename, opt);
            };

            for(auto& filename : filenames)
            {
                entries[filename] = nullptr;
            }

            std::vector<std::thread> threads;
            for(auto& [filename, object] : entries)
            {
                threads.emplace_back(std::thread(readFile, std::ref(filename), options, std::ref(object)));
            }

            for(auto& thread : threads)
            {
                thread.join();
            }
            break;
        }

        case(ThreadPool) :
        {
            std::atomic_uint readLeftToComplete = filenames.size();

            // set up the entries container for operations to write to.
            for(auto& filename : filenames)
            {
                entries[filename] = nullptr;
            }

            // add operations
            for(auto& [filename, object] : entries)
            {
                operationQueue->add(ref_ptr<Operation>(new Operation(filename, options, object, readLeftToComplete)));
            }

            // use this thread to read the files as well
            while(ref_ptr<Operation> operation = operationQueue->take())
            {
                operation->object = vsg::read(operation->filename, operation->options);
                --(operation->readLeftToComplete);
            }

            // spinlock wait for the read operations to complete.
            while(readLeftToComplete != 0)
            {
                std::this_thread::yield();
            }

            break;
        }
    }

    auto vsg_loadTime = std::chrono::duration<double, std::chrono::milliseconds::period>(std::chrono::steady_clock::now() - before_vsg_load).count();
    std::cout<<"After batch load() time =  "<<vsg_loadTime<<std::endl;

// clean up thread pool
    if (threadingModel == ThreadPool)
    {
        active->active = false;

        for(auto& thread : poolThreads)
        {
            thread.join();
        }
    }
// end of clean of thread pool

    return entries;
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
