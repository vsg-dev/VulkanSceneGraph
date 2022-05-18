#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Group.h>

namespace vsg
{

    /// Transform node is a pure virtual base class for positioning/scaling/rotation subgraphs.
    class VSG_DECLSPEC Transform : public Inherit<Group, Transform>
    {
    public:
        Transform();

        /// Specify whether the subgraph below this Transform contains nodes that will be culled against the view frustum, such as LOD and CullGroup nodes.
        /// if the subgraph contains no nodes associated with culling then setting subgraphRequiresLocalFrustum to false will allow the RecordTraversal to skip
        /// transforming the view frustum polytope into the local coordinate frame and save time.
        bool subgraphRequiresLocalFrustum = true;

        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        /// Return the transform matrix, multiplying local transform matrix against the matrix passed into the transform(,,) method.
        /// Typically one pre multiplies local transform against the matrix passed in, which during a RecordTraversal will be the previous modelview matrix inherited from above.
        virtual dmat4 transform(const dmat4& mv) const = 0;

    protected:
    };
    VSG_type_name(vsg::Transform);

} // namespace vsg
