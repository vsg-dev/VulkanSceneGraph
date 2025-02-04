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
#include <vsg/maths/transform.h>
#include <vsg/state/StateCommand.h>
#include <vsg/utils/Instrumentation.h>

namespace vsg
{

    class SharedObjects;
    class ReaderWriter;
    class OperationThreads;
    class CommandLine;
    class ShaderSet;
    class FindDynamicObjects;
    class PropagateDynamicObjects;

    using ReaderWriters = std::vector<ref_ptr<ReaderWriter>>;

    /// Class for passing IO related options to vsg::read/write calls.
    class VSG_DECLSPEC Options : public Inherit<Object, Options>
    {
    public:
        Options();
        Options(const Options& rhs, const CopyOp& copyop = {});

        template<typename... Args>
        explicit Options(Args... args)
        {
            (add(args), ...);
        }

        Options& operator=(const Options& rhs) = delete;

        /// read command line options, assign values to this options object to later use with reading/writing files
        virtual bool readOptions(CommandLine& arguments);

        void add(ref_ptr<ReaderWriter> rw = {});
        void add(const ReaderWriters& rws);

        ref_ptr<SharedObjects> sharedObjects;
        ReaderWriters readerWriters;
        ref_ptr<OperationThreads> operationThreads;

        /// Hint to use when searching for Paths with vsg::findFile(filename, options);
        enum FindFileHint
        {
            CHECK_ORIGINAL_FILENAME_EXISTS_FIRST, /// check the filename exists with its original path before trying to find it in Options::paths.
            CHECK_ORIGINAL_FILENAME_EXISTS_LAST,  /// check the filename exists with its original path after failing to find it in Options::paths.
            ONLY_CHECK_PATHS                      /// only check the filename exists in the Options::paths
        };
        FindFileHint checkFilenameHint = CHECK_ORIGINAL_FILENAME_EXISTS_FIRST;

        Paths paths;

        using FindFileCallback = std::function<Path(const Path& filename, const Options* options)>;
        FindFileCallback findFileCallback;

        Path fileCache;

        Path extensionHint;
        bool mapRGBtoRGBAHint = true;

        /// Coordinate convention to use for scene graph
        CoordinateConvention sceneCoordinateConvention = CoordinateConvention::Z_UP;

        /// Coordinate convention to assume for specified lower case file formats extensions
        std::map<Path, CoordinateConvention> formatCoordinateConventions;

        /// User defined ShaderSet map, loaders should check the available ShaderSet using the name of the type of ShaderSet.
        /// Standard names are :
        ///     "pbr" will substitute for vsg::createPhysicsBasedRenderingShaderSet()
        ///     "phong" will substitute for vsg::createPhongShaderSet()
        ///     "flat" will substitute for vsg::createFlatShadedShaderSet()
        ///     "text" will substitute for vsg::createTextShaderSet()
        std::map<std::string, ref_ptr<ShaderSet>> shaderSets;

        /// specification of any StateCommands that will be provided the parents of any newly created subgraphs
        /// scene graph creation routines can use the inherited state information to avoid setting state in the local subgraph.
        StateCommands inheritedState;

        /// Hook for assigning Instrumentation to enable profiling of record traversal.
        ref_ptr<Instrumentation> instrumentation;

        /// mechanism for finding dynamic objects in loaded scene graph
        ref_ptr<FindDynamicObjects> findDynamicObjects;

        /// mechanism for propogating dynamic objects classification up parental chain so that cloning is done on all dynamic objects to avoid sharing of dyanmic parts.
        ref_ptr<PropagateDynamicObjects> propagateDynamicObjects;

    public:
        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return Options::create(*this, copyop); }
        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~Options();
    };
    VSG_type_name(vsg::Options);

    /// convenience function that if a filename has a path, it duplicates the supplied Options object and prepends the path to the new Options::paths, otherwise returns the original Options object.
    extern VSG_DECLSPEC ref_ptr<const vsg::Options> prependPathToOptionsIfRequired(const vsg::Path& filename, ref_ptr<const vsg::Options> options);

    /// return true if the options->extensionHint or filename extension are found in the list of arguments/containers
    template<typename... Args>
    bool compatibleExtension(const vsg::Path& filename, const vsg::Options* options, const Args&... args)
    {
        if (options && options->extensionHint && contains(options->extensionHint, args...)) return true;
        return contains(vsg::lowerCaseFileExtension(filename), args...);
    }

    /// return true if the options->extensionHint is found in the list of arguments/containers
    template<typename... Args>
    bool compatibleExtension(const vsg::Options* options, const Args&... args)
    {
        return options && options->extensionHint && contains(options->extensionHint, args...);
    }

} // namespace vsg
