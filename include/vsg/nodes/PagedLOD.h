#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/FileSystem.h>
#include <vsg/io/Options.h>
#include <vsg/nodes/Node.h>
#include <vsg/vk/Semaphore.h>

#include <array>

namespace vsg
{

    // forward declare
    class PagedLODList;

    /** Level of Detail Node,
     *  Children should be ordered with the highest resolution PagedLODChild first, through to lowest resolution PagedLODChild last.
     *  The PagedLODChild struct stores the minimumScreenHeightRatio and child that it's associated with.
     *  During culling the minimumScreenHeightRatio is used as a minimum ratio of screen height that a bounding sphere needs to occupy in order for the associated child to be traversed.
     *  Once one child passes this test no more children are checked, so that no more than one child will ever be traversed in a record traversal.
     *  If no PagedLODChild passes the visible height test then none of the PagedLOD's children will be visible.
     *  During the record traversals the Bound sphere is also checked against the view frustum so that PagedLOD's also enable view frustum culling for subgraphs so there is no need for a separate CullNode/CullGroup to decorate it. */
    class VSG_DECLSPEC PagedLOD : public Inherit<Node, PagedLOD>
    {
    public:
        PagedLOD();
        PagedLOD(const PagedLOD& rhs, const CopyOp& copyop = {});

        struct Child
        {
            double minimumScreenHeightRatio = 0.0; // 0.0 is always visible
            ref_ptr<Node> node;
        };

        // external file to load when child 0 is null.
        Path filename;

        dsphere bound;

        using Children = std::array<Child, 2>;
        Children children;

        bool highResActive(uint64_t frameCount, uint64_t inactiveAge = 3) const
        {
            return (frameCount - frameHighResLastUsed.load()) <= inactiveAge;
        }

    public:
        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return PagedLOD::create(*this, copyop); }
        int compare(const Object& rhs) const override;

        template<class N, class V>
        static void t_traverse(N& node, V& visitor)
        {
            for (auto& child : node.children)
            {
                if (child.node) child.node->accept(visitor);
            }
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }
        void traverse(RecordTraversal& visitor) const override { t_traverse(*this, visitor); }

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~PagedLOD();

    public:
        ref_ptr<Options> options;

        // priority value assigned by record traversal as a guide to how important the external child is for loading.
        mutable std::atomic<double> priority{0.0};

        mutable std::atomic_uint64_t frameHighResLastUsed{0};
        mutable std::atomic_uint requestCount{0};

        enum RequestStatus : unsigned int
        {
            NoRequest = 0,
            ReadRequest = 1,
            Reading = 2,
            Compiling = 3,
            MergeRequest = 4,
            Merging = 5,
            DeleteRequest = 6,
            Deleting = 7
        };

        mutable std::atomic<RequestStatus> requestStatus{NoRequest};
        mutable uint32_t index = 0;

        ref_ptr<Node> pending;
    };
    VSG_type_name(vsg::PagedLOD);

    struct PagedLODContainer : public Inherit<Object, PagedLODContainer>
    {
        explicit PagedLODContainer(uint32_t maxNumPagedLOD = 10000);

        struct List
        {
            uint32_t head = 0;
            uint32_t tail = 0;
            uint32_t count = 0;
            std::string name;
        };

        struct Element
        {
            uint32_t previous = 0;
            uint32_t next = 0;
            ref_ptr<PagedLOD> plod;
            List* list = nullptr;
        };

        using Elements = std::vector<Element>;

        Elements elements;
        List availableList;
        List inactiveList;
        List activeList;

        void resize(uint32_t new_size);
        void resize();
        void inactive(const PagedLOD* plod);
        void active(const PagedLOD* plod);
        void remove(PagedLOD* plod);

        void _move(const PagedLOD* plod, List* targetList);

        bool check();
        bool check(const List& list);

        void print(std::ostream& out);
    };

} // namespace vsg
