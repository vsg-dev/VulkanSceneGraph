#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield
Copyright(c) 2020 Julien Valentin

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Device.h>

namespace vsg
{
    class SubPass;
    class Dependency;
    ///
    /// @brief Directed Acyclic Graph managing SubPasses and Dependencies
    ///
    class VSG_DECLSPEC PassGraph : public Inherit<Object, PassGraph>
    {
    public:
        PassGraph() {}
        friend class SubPass;
        using Dependencies = std::vector< ref_ptr<Dependency> >;
        using SubPasses = std::vector< ref_ptr<SubPass> >;

        struct AttachmentDescription : public VkAttachmentDescription
        {
            AttachmentDescription()
            {
                samples= VK_SAMPLE_COUNT_1_BIT;
                loadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                storeOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                initialLayout= VK_IMAGE_LAYOUT_UNDEFINED;
                finalLayout= VK_IMAGE_LAYOUT_UNDEFINED;
            }
            bool operator ==(const AttachmentDescription& other) const
            {
                return(flags==other.flags
                       && format==other.format
                       && samples==other.samples
                       && loadOp==other.loadOp
                       && storeOp==other.storeOp
                       && stencilLoadOp==other.stencilLoadOp
                       && stencilStoreOp==other.stencilStoreOp
                       && initialLayout==other.initialLayout
                       && finalLayout==other.finalLayout);
            }
        };
        using AttachmentDescriptions = std::vector< AttachmentDescription >;

        void setDepthFormat(VkFormat d) { _depthFormat = d;  }
        void setColorFormat(VkFormat d) { _imageFormat = d;  }

        inline SubPass* createSubPass();
        inline void removeSubPass(SubPass* v);

        const SubPass* getSubPass(int i) const { return _subpasses[i]; }
        SubPass* getSubPass(int i) { return _subpasses[i]; }
        uint getNumSubPasses() const { return _subpasses.size(); }
        inline int getSubPassIndex(SubPass* v) const;

        inline void addDependency(Dependency* v) { _depends.push_back(vsg::ref_ptr<Dependency> (v)); }
        inline void removeDependency(Dependency* v);

        const Dependency* getDependency(int i) const { return _depends[i]; }
        Dependency* getDependency(int i) { return _depends[i]; }
        uint getNumDependencies() const {  return _depends.size(); }

        inline void addAttachmentDescription(AttachmentDescription &v) { _attachments.push_back(v); }
        inline void removeAttachmentDescription(AttachmentDescription &v);

        const AttachmentDescription& getAttachmentDescription(int i) const { return _attachments[i]; }
        AttachmentDescription& getAttachmentDescription(int i) { return _attachments[i]; }
        inline int getAttachmentIndex(AttachmentDescription& v) const;

        uint getNumAttachmentDescriptions() const { return _attachments.size(); }
        AttachmentDescriptions & getAttachmentDescriptions() { return _attachments; }

    protected:
        inline void addSubPass(SubPass* v);
        inline bool addInputAttachment(SubPass* sub, AttachmentDescription& ad, VkImageLayout l);
        inline bool addColorAttachment(SubPass* sub, AttachmentDescription& ad, VkImageLayout l);
        inline bool addDepthStencilAttachment(SubPass* sub, AttachmentDescription& ad, VkImageLayout l);
        inline bool addResolveAttachment(SubPass* sub, AttachmentDescription& ad, VkImageLayout l);

        AttachmentDescriptions _attachments;
        ref_ptr<Device> _device;
        ref_ptr<AllocationCallbacks> _allocator;
        VkFormat _depthFormat, _imageFormat;
        SubPasses _subpasses;
        Dependencies _depends;

    };

    ///Render pass composite
    class VSG_DECLSPEC SubPass : public Inherit<Object, SubPass> {
    public:
        friend class PassGraph;
        using AttachmentRef = std::pair< PassGraph::AttachmentDescription, VkImageLayout>;
        using Dependencies = std::vector< ref_ptr<Dependency> >;
        struct AttachmentReference : public VkAttachmentReference
        {
            AttachmentReference() { attachment = 0; layout = VK_IMAGE_LAYOUT_UNDEFINED; }

            bool operator ==(const AttachmentReference& other) const
            {
                return(attachment == other.attachment && layout == other.layout);
            }
        };


        void setBindPoint(VkPipelineBindPoint b = VK_PIPELINE_BIND_POINT_GRAPHICS)
        {
            if(b == _desc.pipelineBindPoint) return;
            _desc.pipelineBindPoint = b;
        }

        inline const PassGraph * getPassGraph() const { return _graph; }
        inline PassGraph * getPassGraph() { return _graph; }

        bool addInputAttachmentRef(PassGraph::AttachmentDescription & ad, VkImageLayout l)
        {
            if(_graph) return _graph->addInputAttachment(this, ad, l);
            return false;
        }
        bool addDepthStencilAttachmentRef(PassGraph::AttachmentDescription & ad, VkImageLayout l)
        {
            if(_graph) return _graph->addDepthStencilAttachment(this, ad, l);
            return false;
        }
        bool addResolvAttachmentRef(PassGraph::AttachmentDescription & ad, VkImageLayout l)
        {
            if(_graph) return _graph->addResolveAttachment(this, ad, l);
            return false;
        }
        bool addColorAttachmentRef(PassGraph::AttachmentDescription & ad, VkImageLayout l)
        {
            if(_graph) return _graph->addColorAttachment(this, ad, l);
            return false;
        }
        inline void removeColorAttachmentRef(AttachmentReference &v)
        {
            for (auto item =_colorattachmentrefs.begin(); item != _colorattachmentrefs.end(); ++item ) if(*item==v) { _colorattachmentrefs.erase(item); return; }
        }

        const AttachmentReference& getColorAttachmentRef(int i) const { return _colorattachmentrefs[i]; }
        AttachmentReference& getColorAttachmentRef(int i) { return _colorattachmentrefs[i]; }
        uint getNumColorAttachmentRefs() const { return _colorattachmentrefs.size(); }
        uint getNumInputAttachmentRefs() const { return _inputattachmentrefs.size(); }

        ///Dependencies management
        inline Dependency * createForwardDependency(SubPass * next = nullptr);
        inline Dependency * createBackwardDependency(SubPass * prev = nullptr);
        inline void getForwardDependencies(Dependencies&);
        inline void getBackWardDependencies(Dependencies&);

        //validate Attachment Refs and return it
        operator VkSubpassDescription ()
        {
            _desc.colorAttachmentCount=_colorattachmentrefs.size();
            _desc.pColorAttachments=_colorattachmentrefs.data();
            _desc.inputAttachmentCount=_inputattachmentrefs.size();
            _desc.pInputAttachments=_colorattachmentrefs.data();
            _desc.inputAttachmentCount=_inputattachmentrefs.size();
            _desc.pInputAttachments=_colorattachmentrefs.data();
            _desc.pDepthStencilAttachment = &_pDepthStencilAttachment;
            return _desc;
        }

    protected:

        ///constructor
        SubPass(PassGraph * graph) : _graph(graph) {}
        VkSubpassDescription _desc = {};
        std::vector<AttachmentReference> _inputattachmentrefs;
        std::vector<AttachmentReference> _colorattachmentrefs;
        std::vector<AttachmentReference> _pResolveAttachments;
        AttachmentReference _pDepthStencilAttachment;
        std::vector<uint> _pPreserveAttachments;
        ref_ptr<PassGraph> _graph;
    };

    class VSG_DECLSPEC Dependency : public Inherit<Object, Dependency> {
    public:
        friend class SubPass;
        Dependency(SubPass* src = nullptr, SubPass* dst = nullptr): _src(src), _dst(dst)
        {
            _desc.srcSubpass = VK_SUBPASS_EXTERNAL;
            _desc.dstSubpass = VK_SUBPASS_EXTERNAL;
            _desc.srcAccessMask = 0;
            _desc.srcAccessMask = 0;
            _desc.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            _desc.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            _desc.dependencyFlags = 0;
        }
        inline void setDstAccessMask(VkAccessFlags d) { _desc.dstAccessMask = d; }
        inline void setSrcAccessMask(VkAccessFlags d) { _desc.srcAccessMask = d; }
        inline void setSrcStageMask(VkPipelineStageFlags d) { _desc.srcStageMask = d; }
        inline void setDstStageMask(VkPipelineStageFlags d) { _desc.dstStageMask = d; }
        inline void setDependencyFlags(VkDependencyFlags d) { _desc.dependencyFlags = d; }

        //validate Attachment Refs and return it
        operator VkSubpassDependency ()
        {
            int isrc = -1, idst = -1;
            if(_src)
            {
                isrc = _src->getPassGraph()->getSubPassIndex(_src);
                _desc.srcSubpass = isrc;
            }
            if(_dst)
            {
                idst = _dst->getPassGraph()->getSubPassIndex(_dst);
                //TODO ERROR if(isrc<0||idst<0)
                _desc.dstSubpass = idst;
            }
            return _desc;
        }
    protected:
        ref_ptr<SubPass> _src,  _dst;
        VkSubpassDependency _desc = {};
    };

    bool PassGraph::addInputAttachment(SubPass* sub, AttachmentDescription & ad, VkImageLayout l)
    {
        SubPass::AttachmentReference attachref;
        attachref.layout=l; int attindex;
        if((attindex = getAttachmentIndex(ad)) < 0)
        {
            attachref.attachment = _attachments.size();
            addAttachmentDescription(ad);
        }
        else attachref.attachment = attindex;
        sub->_inputattachmentrefs.push_back(attachref);
        return true;
    }

    bool PassGraph::addColorAttachment(SubPass* sub, AttachmentDescription & ad, VkImageLayout l)
    {
        SubPass::AttachmentReference attachref = {};
        attachref.layout=l; int attindex;
        if((attindex = getAttachmentIndex(ad)) < 0)
        {
            attachref.attachment = _attachments.size();
            addAttachmentDescription(ad);
        }
        else attachref.attachment = attindex;
        sub->_colorattachmentrefs.push_back(attachref);
        return true;
    }

    bool PassGraph::addDepthStencilAttachment(SubPass* sub, AttachmentDescription & ad, VkImageLayout l)
    {
        SubPass::AttachmentReference attachref = {};
        attachref.layout=l; int attindex;
        if((attindex=getAttachmentIndex(ad)) < 0)
        {
            attachref.attachment = _attachments.size();
            addAttachmentDescription(ad);
        }
        else attachref.attachment = attindex;
        sub->_pDepthStencilAttachment = attachref;
        return true;
    }

    bool PassGraph::addResolveAttachment(SubPass* sub, AttachmentDescription & ad, VkImageLayout l)
    {
        SubPass::AttachmentReference attachref = {};
        attachref.layout=l; int attindex;
        if((attindex=getAttachmentIndex(ad)) < 0)
        {
            attachref.attachment = _attachments.size();
            addAttachmentDescription(ad);
        }
        else attachref.attachment = attindex;
        sub->_pResolveAttachments.push_back(attachref);
        return true;
    }

    void SubPass::getForwardDependencies(SubPass::Dependencies& ret)
    {
        if(_graph)
        {
            for(uint i=0; i<_graph->getNumDependencies(); ++i)
                if(_graph->getDependency(i)->_src == this) ret.push_back(ref_ptr<Dependency>(_graph->getDependency(i)));
        }
    }

    void SubPass::getBackWardDependencies(SubPass::Dependencies& ret)
    {
        if(_graph)
        {
            for(uint i=0; i<_graph->getNumDependencies(); ++i)
                if(_graph->getDependency(i)->_dst == this) ret.push_back(ref_ptr<Dependency>(_graph->getDependency(i)));
        }
    }

    Dependency * SubPass::createForwardDependency(SubPass * next)
    {
        if(_graph)
        {
            Dependency *dep = new Dependency(this, next);
            _graph->addDependency(dep);
            return dep;
        }
        return nullptr;
    }

    Dependency * SubPass::createBackwardDependency(SubPass * prev)
    {
        if(_graph)
        {
            Dependency *dep = new Dependency(prev, this);
            _graph->addDependency(dep);
            return dep;
        }
        return nullptr;
    }

    SubPass* PassGraph::createSubPass() { ref_ptr<SubPass> sub(new SubPass(this)); _subpasses.push_back((sub)); return sub; }
    int PassGraph::getSubPassIndex(SubPass* v) const
    {
        int cpt = 0;
        for (auto item =_subpasses.begin(); item != _subpasses.end(); ++item, ++cpt) if(*item == v) return cpt;
        return -1;
    }

    void PassGraph::addSubPass(SubPass* v)
    {
        if(!v) return;
        _subpasses.push_back(vsg::ref_ptr<SubPass> (v));

    }
    void PassGraph::removeSubPass(SubPass* v)
    {
        for (auto item =_subpasses.begin(); item != _subpasses.end(); ++item) if(*item==v)
        {
                _subpasses.erase(item);

        }
    }
    void  PassGraph::removeAttachmentDescription(AttachmentDescription &v)
    {
        for (auto item =_attachments.begin(); item != _attachments.end(); ++item ) if(*item==v) _attachments.erase(item);
    }

    void PassGraph::removeDependency(Dependency* v)
    {
        for (auto item =_depends.begin(); item != _depends.end(); ++item) if(*item == v) { _depends.erase(item); return;}
    }

    int PassGraph::getAttachmentIndex( AttachmentDescription&v) const
    {
        uint cpt=0;
        for (auto item =_attachments.begin(); item != _attachments.end(); ++item,++cpt) if(*item==v) return cpt;
        return -1;
    }
} // namespace vsg
