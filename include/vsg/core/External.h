#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>

#include <vsg/io/FileSystem.h>

namespace vsg
{

    /// External provides a means to loading objects from external files, such as shaders, textures or models
    /// To use set up the External object with all the pairs of [filename, object] that should be managed externally
    /// then assign the External object, as use value, to root node of the scene graph that you wish to use external objects with
    /// so that when serializing the External object is initialized and external objects are loaded before they are
    /// needed by the rest of the subgraph i.e.
    ///     auto external = vsg::External::create("mytexture.png", texture);
    ///     scene->setObject("external", external); // scene uses the texture object somewhere within it.
    class VSG_DECLSPEC External : public Inherit<Object, External>
    {
    public:
        External();
        explicit External(const PathObjects& entries);
        External(const vsg::Path& filename, ref_ptr<Object> object);

        template<class O, class V>
        static void t_traverse(O& object, V& visitor)
        {
            for (auto itr = object.entries.begin(); itr != object.entries.end(); ++itr)
            {
                if (itr->second) itr->second->accept(visitor);
            }
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }
        void traverse(RecordTraversal&) const override {}

        void read(Input& input) override;
        void write(Output& output) const override;

        /// custom readwriter/writer options
        ref_ptr<Options> options;

        /// list of path/object pairs
        PathObjects entries;

        void add(const Path& filename, ref_ptr<Object> object = {}) { entries[filename] = object; }

    protected:
        virtual ~External();
    };
    VSG_type_name(vsg::External);

} // namespace vsg
