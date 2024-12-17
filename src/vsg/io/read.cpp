/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/VSG.h>
#include <vsg/io/glsl.h>
#include <vsg/io/read.h>
#include <vsg/io/spirv.h>
#include <vsg/io/tile.h>
#include <vsg/io/txt.h>
#include <vsg/threading/OperationThreads.h>
#include <vsg/utils/FindDynamicObjects.h>
#include <vsg/utils/PropagateDynamicObjects.h>
#include <vsg/utils/SharedObjects.h>

using namespace vsg;

ref_ptr<Object> vsg::read(const Path& filename, ref_ptr<const Options> options)
{
    CPU_INSTRUMENTATION_L1_NC(options ? options->instrumentation.get() : nullptr, "read", COLOR_READ);

    auto read_file = [&]() -> ref_ptr<Object> {
        if (options && !options->readerWriters.empty())
        {
            for (auto& readerWriter : options->readerWriters)
            {
                auto object = readerWriter->read(filename, options);
                if (object) return object;
            }
            return {};
        }

        auto ext = vsg::lowerCaseFileExtension(filename);

        if (ext == ".vsga" || ext == ".vsgt" || ext == ".vsgb")
        {
            VSG rw;
            return rw.read(filename, options);
        }
        else if (ext == ".spv")
        {
            spirv rw;
            return rw.read(filename, options);
        }
        else if (glsl::extensionSupported(ext))
        {
            glsl rw;
            return rw.read(filename, options);
        }
        else if (txt::extensionSupported(ext))
        {
            txt rw;
            return rw.read(filename, options);
        }
        else
        {
            // no means of loading file
            return {};
        }
    };

    if (options && options->sharedObjects && options->sharedObjects->suitable(filename))
    {
        auto loadedObject = LoadedObject::create(filename, options);

        options->sharedObjects->share(loadedObject, [&](auto load) {
            load->object = read_file();

            if (load->object && options && options->findDynamicObjects && options->propagateDynamicObjects)
            {
                // invoke the find and propagate visitors to collate all the dynamic objects that will need to be cloned.

                std::scoped_lock<std::mutex> fdo_lock(options->findDynamicObjects->mutex);

                options->findDynamicObjects->dynamicObjects.clear();
                load->object->accept(*(options->findDynamicObjects));

                std::scoped_lock<std::mutex> pdo_lock(options->propagateDynamicObjects->mutex);
                options->propagateDynamicObjects->dynamicObjects.swap(options->findDynamicObjects->dynamicObjects);
                load->object->accept(*(options->propagateDynamicObjects));

                load->dynamicObjects.swap(options->propagateDynamicObjects->dynamicObjects);
            }
        });

        if (!loadedObject->dynamicObjects.empty())
        {
            vsg::CopyOp copyop;
            auto duplicate = copyop.duplicate = new vsg::Duplicate;
            for (auto& object : loadedObject->dynamicObjects)
            {
                duplicate->insert(object);
            }

            vsg::info("loaded filename = ", filename, ", object = ", loadedObject->object, ", dynamicObjects.size() = ", loadedObject->dynamicObjects.size());

            return copyop(loadedObject->object);
        }

        return loadedObject->object;
    }
    else
    {
        return read_file();
    }
}

PathObjects vsg::read(const Paths& filenames, ref_ptr<const Options> options)
{
    CPU_INSTRUMENTATION_L1_NC(options ? options->instrumentation.get() : nullptr, "read", COLOR_READ);

    ref_ptr<OperationThreads> operationThreads;
    if (options) operationThreads = options->operationThreads;

    PathObjects entries;

    if (operationThreads && filenames.size() > 1)
    {
        // set up the entries container for operations to write to.
        for (const auto& filename : filenames)
        {
            entries[filename] = nullptr;
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

        // use latch to synchronize this thread with the file reading threads
        auto latch = Latch::create(static_cast<int>(filenames.size()));

        // add operations
        for (auto& [filename, object] : entries)
        {
            operationThreads->add(ref_ptr<Operation>(new ReadOperation(filename, options, object, latch)));
        }

        // use this thread to read the files as well
        operationThreads->run();

        // wait till all the read operations have completed
        latch->wait();
    }
    else
    {
        // run reads single threaded
        for (auto& filename : filenames)
        {
            if (filename)
            {
                entries[filename] = vsg::read(filename, options);
            }
            else
            {
                entries[filename] = nullptr;
            }
        }
    }

    return entries;
}

ref_ptr<Object> vsg::read(std::istream& fin, ref_ptr<const Options> options)
{
    CPU_INSTRUMENTATION_L1_NC(options ? options->instrumentation.get() : nullptr, "read", COLOR_READ);

    if (options && !options->readerWriters.empty())
    {
        for (auto& readerWriter : options->readerWriters)
        {
            auto object = readerWriter->read(fin, options);
            if (object) return object;
        }
    }

    return {};
}

ref_ptr<Object> vsg::read(const uint8_t* ptr, size_t size, ref_ptr<const Options> options)
{
    CPU_INSTRUMENTATION_L1_NC(options ? options->instrumentation.get() : nullptr, "read", COLOR_READ);

    if (options && !options->readerWriters.empty())
    {
        for (auto& readerWriter : options->readerWriters)
        {
            auto object = readerWriter->read(ptr, size, options);
            if (object) return object;
        }
    }

    return {};
}
