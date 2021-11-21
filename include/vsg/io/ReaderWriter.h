#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>
#include <vsg/io/FileSystem.h>
#include <vsg/io/Options.h>

namespace vsg
{

    struct ReadError : public Inherit<Object, ReadError>
    {
        ReadError(const std::string& msg) :
            message(msg) {}

        std::string message;
    };

    struct WriteError : public Inherit<Object, WriteError>
    {
        WriteError(const std::string& msg) :
            message(msg) {}

        std::string message;
    };

    class VSG_DECLSPEC ReaderWriter : public Inherit<Object, ReaderWriter>
    {
    public:
        using vsg::Object::read;
        using vsg::Object::write;

        /// convenience method for casting a read object to a specified type.
        template<class T>
        vsg::ref_ptr<T> read_cast(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options = {}) const
        {
            auto object = read(filename, options);
            return vsg::ref_ptr<T>(dynamic_cast<T*>(object.get()));
        }

        /// convenience method for casting a read object to a specified type.
        template<class T>
        vsg::ref_ptr<T> read_cast(std::istream& fin, vsg::ref_ptr<const vsg::Options> options = {}) const
        {
            auto object = read(fin, options);
            return vsg::ref_ptr<T>(dynamic_cast<T*>(object.get()));
        }

        /// read object from specified file, return object on success, return null ref_ptr<> on failure.
        virtual vsg::ref_ptr<vsg::Object> read(const vsg::Path& /*filename*/, vsg::ref_ptr<const vsg::Options> = {}) const { return vsg::ref_ptr<vsg::Object>(); }
        virtual vsg::ref_ptr<vsg::Object> read(std::istream& /*fin*/, vsg::ref_ptr<const vsg::Options> = {}) const { return vsg::ref_ptr<vsg::Object>(); }
        virtual vsg::ref_ptr<vsg::Object> read(const uint8_t* /*ptr*/, size_t /*size*/, vsg::ref_ptr<const vsg::Options> = {}) const { return vsg::ref_ptr<vsg::Object>(); }

        /// write object to specified file, return true on success, return false on failure.
        virtual bool write(const vsg::Object* /*object*/, const vsg::Path& /*filename*/, vsg::ref_ptr<const vsg::Options> = {}) const { return false; }
        virtual bool write(const vsg::Object* /*object*/, std::ostream& /*fout*/, vsg::ref_ptr<const vsg::Options> = {}) const { return false; }

        /// read the command line arguments for any options appropriate for this ReaderWriter
        virtual bool readOptions(Options&, CommandLine&) const { return false; }

        enum class FeatureMask : uint8_t
        {
            READ_FILENAME = (1 << 0),
            READ_ISTREAM = (1 << 1),
            READ_MEMORY = (1 << 2),
            WRITE_FILENAME = (1 << 3),
            WRITE_OSTREAM = (1 << 4)
        };

        struct Features
        {
            std::map<std::string, FeatureMask> protocolFeatureMap;
            std::map<std::string, FeatureMask> extensionFeatureMap;
        };

        /// get the Features supported by this ReaderWriter
        virtual bool getFeatures(Features&) const { return false; }
    };
    VSG_type_name(vsg::ReaderWriter);

    class VSG_DECLSPEC CompositeReaderWriter : public Inherit<ReaderWriter, CompositeReaderWriter>
    {
    public:
        using ReaderWriters = std::vector<vsg::ref_ptr<ReaderWriter>>;
        ReaderWriters readerWriters;

        void add(ref_ptr<ReaderWriter> reader);

        void read(Input& input) override;
        void write(Output& output) const override;

        vsg::ref_ptr<vsg::Object> read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options = {}) const override;
        vsg::ref_ptr<vsg::Object> read(std::istream& fin, vsg::ref_ptr<const vsg::Options> options = {}) const override;
        vsg::ref_ptr<vsg::Object> read(const uint8_t* ptr, size_t size, vsg::ref_ptr<const vsg::Options> options = {}) const override;

        bool write(const vsg::Object* object, const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options = {}) const override;
        bool write(const vsg::Object* object, std::ostream& fout, vsg::ref_ptr<const vsg::Options> options = {}) const override;

        /// read the command line arguments for any options appropriate for this ReaderWriter
        bool readOptions(vsg::Options& options, vsg::CommandLine& arguments) const override;

        bool getFeatures(Features& features) const override;

    protected:
    };
    VSG_type_name(vsg::CompositeReaderWriter);

} // namespace vsg
