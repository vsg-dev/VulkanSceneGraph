/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/io/ReaderWriter.h>
#include <vsg/state/DescriptorSetLayout.h>
#include <vsg/threading/OperationThreads.h>
#include <vsg/utils/CommandLine.h>
#include <vsg/utils/FindDynamicObjects.h>
#include <vsg/utils/PropagateDynamicObjects.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/utils/SharedObjects.h>

using namespace vsg;

Options::Options()
{
    getOrCreateAuxiliary();

    formatCoordinateConventions[".gltf"] = CoordinateConvention::Y_UP;
    formatCoordinateConventions[".glb"] = CoordinateConvention::Y_UP;
    formatCoordinateConventions[".dae"] = CoordinateConvention::Y_UP;
    formatCoordinateConventions[".stl"] = CoordinateConvention::NO_PREFERENCE;
    formatCoordinateConventions[".obj"] = CoordinateConvention::NO_PREFERENCE;

    findDynamicObjects = FindDynamicObjects::create();
    propagateDynamicObjects = PropagateDynamicObjects::create();
}

Options::Options(const Options& options, const CopyOp& copyop) :
    Inherit(options, copyop),
    sharedObjects(options.sharedObjects),
    readerWriters(options.readerWriters),
    operationThreads(options.operationThreads),
    checkFilenameHint(options.checkFilenameHint),
    paths(options.paths),
    findFileCallback(options.findFileCallback),
    fileCache(options.fileCache),
    extensionHint(options.extensionHint),
    mapRGBtoRGBAHint(options.mapRGBtoRGBAHint),
    sceneCoordinateConvention(options.sceneCoordinateConvention),
    formatCoordinateConventions(options.formatCoordinateConventions),
    shaderSets(options.shaderSets),
    inheritedState(options.inheritedState),
    instrumentation(options.instrumentation),
    findDynamicObjects(options.findDynamicObjects),
    propagateDynamicObjects(options.propagateDynamicObjects)
{
    getOrCreateAuxiliary();
    // copy any meta data.
    if (options.getAuxiliary()) getAuxiliary()->userObjects = options.getAuxiliary()->userObjects;
}

Options::~Options()
{
}

int Options::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_pointer_container(readerWriters, rhs.readerWriters))) return result;
    if ((result = compare_value(checkFilenameHint, rhs.checkFilenameHint))) return result;
    if ((result = compare_container(paths, rhs.paths))) return result;
    if ((result = compare_value(fileCache, rhs.fileCache))) return result;
    if ((result = compare_value(extensionHint, rhs.extensionHint))) return result;
    if ((result = compare_value(mapRGBtoRGBAHint, rhs.mapRGBtoRGBAHint))) return result;
    if ((result = compare_value(sceneCoordinateConvention, rhs.sceneCoordinateConvention))) return result;
    if ((result = compare_value(formatCoordinateConventions, rhs.formatCoordinateConventions))) return result;
    return compare_value(shaderSets, rhs.shaderSets);
}

void Options::read(Input& input)
{
    Object::read(input);

    input.readObject("sharedObjects", sharedObjects);

    readerWriters.clear();
    uint32_t count = input.readValue<uint32_t>("NumReaderWriters");
    for (uint32_t i = 0; i < count; ++i)
    {
        auto rw = input.readObject<ReaderWriter>("ReaderWriter");
        if (rw) readerWriters.push_back(rw);
    }

    input.readObject("operationThreads", operationThreads);
    input.readValue<uint32_t>("checkFilenameHint", checkFilenameHint);

    paths.resize(input.readValue<uint32_t>("NumPaths"));
    for (auto& path : paths)
    {
        input.read("path", path);
    }

    input.read("fileCache", fileCache);
    input.read("extensionHint", extensionHint);
    input.read("mapRGBtoRGBAHint", mapRGBtoRGBAHint);

    shaderSets.clear();
    uint32_t numShaderSets = input.readValue<uint32_t>("numShaderSets");
    for (; numShaderSets > 0; --numShaderSets)
    {
        std::string name;
        ref_ptr<ShaderSet> shaderSet;
        input.read("name", name);
        input.readObject("shaderSet", shaderSet);
        shaderSets[name] = shaderSet;
    }
}

void Options::write(Output& output) const
{
    Object::write(output);

    output.writeObject("sharedObjects", sharedObjects);

    output.writeValue<uint32_t>("NumReaderWriters", readerWriters.size());
    for (const auto& rw : readerWriters)
    {
        output.writeObject("ReaderWriter", rw);
    }

    output.writeObject("operationThreads", operationThreads);
    output.writeValue<uint32_t>("checkFilenameHint", checkFilenameHint);

    output.writeValue<uint32_t>("NumPaths", paths.size());
    for (const auto& path : paths)
    {
        output.write("path", path);
    }

    output.write("fileCache", fileCache);
    output.write("extensionHint", extensionHint);
    output.write("mapRGBtoRGBAHint", mapRGBtoRGBAHint);

    output.writeValue<uint32_t>("numShaderSets", shaderSets.size());
    for (auto& [name, shaderSet] : shaderSets)
    {
        output.write("name", name);
        output.writeObject("shaderSet", shaderSet);
    }
}

void Options::add(ref_ptr<ReaderWriter> rw)
{
    if (rw) readerWriters.push_back(rw);
}

void Options::add(const ReaderWriters& rws)
{
    for (const auto& rw : rws) add(rw);
}

bool Options::readOptions(CommandLine& arguments)
{
    bool optionsRead = false;
    for (auto& readerWriter : readerWriters)
    {
        if (readerWriter->readOptions(*this, arguments)) optionsRead = true;
    }

    if (arguments.read("--file-cache", fileCache)) optionsRead = true;
    if (arguments.read("--extension-hint", extensionHint)) optionsRead = true;

    return optionsRead;
}

ref_ptr<const vsg::Options> vsg::prependPathToOptionsIfRequired(const vsg::Path& filename, ref_ptr<const vsg::Options> options)
{
    auto path = filePath(filename);
    if (!path) return options;

    auto duplicate = vsg::Options::create(*options);
    duplicate->paths.insert(duplicate->paths.begin(), path);

    return duplicate;
}
