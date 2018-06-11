#include <vsg/Object.h>
#include <vsg/ref_ptr.h>

// preliminary list of compoents the the vsg libraries and the possible classes that they'll contain
// listed here just to give an overall feel of what will be needed
namespace vsg
{
    namespace core
    {
        class ref_ptr;
        class objsever_ptr;
        class Object;
        class Auxiliary;
        class Visitor;
    }

    namespace maths
    {
        class vec2; // double, float, int, uint etc. variants
        class vec3;
        class vec4;
        class mat4;
        // GLSL style functions like cross, dot, inverse etc.
    }

    namespace nodes
    {
        class Node;
        class Group;
        class GroupN; // fixed sized Groups
        class Cull; // implement view frustum culling - i.e. compare bounding sphere to view frustum
        class LOD; // fixed size of two LOD children, two distances
        class PagedLOD; // fixed size of two LOD children, two distances
        class Transform;
        class Mask;
        class Switch;
        class State; // attach PipelineComponents to the scene graph
        class Draw; // dispatch a graphics call
        class Compute; // dispatch a compute call
        // need some scene graph mechansism for controlling draw order/conrolling the rendering backend
    };

    namespace state
    {
        class PipelineComponent;
        class PipelineComponents; // collection of PipelinComponents
        class PipelineState; // complete set of PipelineConponents to be passed to GL
        class Uniform; // pipline component
        class Array; // pipline component
        class Texture; // pipline component
        class Buffer; // pipline component
        class Program; // pipline component
        class Shader; // pipline component
        class Define; // some sort of #pragma(tic) shader composition
    };

    namespace render
    {
        class RenderBin;  // has map<PipelineState*, std::vector<Draw/Compute> and std::vector<pair<PipelineState*, Draw/Compute> for ordered list.
        class RenderStage; // RenderBin + pre and post preperation
        class StateGraph; // cacche hierarchy of PipleinComponsts to PipelineState
        class CullTraversal; // traverse scene graph to generate rendering graphs
        class DrawTraversal; // traverse rendering graphs to dispatch to vulkan
    }

    namespace compute
    {
        // ?  Need to research what Vulkan does for compute
    }

    namespace viewer
    {
        class Camera; // also a Node, perhaps should be in nodes?
        class View; // master Camera + optional list of slave Camera
        class Viewer; // "has a" list of View (equivilant to CompositeViewer)
        class RenderSurface; // context/buffer object?
        class Compositor; // presentor, take all the rendering output and present on screen
        // some basic event handling?
    }

    namespace introspection
    {
        // some sort of generic API for classes, properties and methods and global functions. make available to C?
        // some sort of object wrapping
    }

    namespace io
    {
        ref_ptr<Object> readFile(std::iostream&);
        ref_ptr<Object> readFile(const std::string& filename);
        void writeFile(Object& object, std::ostream&);
        void writeFile(Object& object, const std::string& filename);

        class DatabasePager;
    }

};

namespace vsg
{
    // for convinience we want all the compoents to be available in the vsg namespace
    using namespace core;
    using namespace nodes;
    using namespace state;
    using namespace render;
    using namespace compute;
    using namespace viewer;
    using namespace introspection;
    using namespace io;
}

int main(int argc, char** argv)
{
    return 0;
};