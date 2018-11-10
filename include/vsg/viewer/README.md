# include/vsg/viewer headers

The include/vsg/viewer header directory contains the Windowing  and Viewer classes required to create a Vulkan window(s) and viewer to render to it/then.

* [include/vsg/viewer/GraphicsStage.h](GraphicsStage.h) - class for managing the dispatch of Vulkan state and geometry data into the Vulkan graphics queue.
* [include/vsg/viewer/Window.h](Window.h) - base class for creation of Windows with Vulkan support.  Subclasses from vsg::Window provide the implementation for the different target platforms.  Currently a GLFW based Window is provided internally by libvsg to server the role as cross platform Window implementation.  Plan is to replace the GLFW version with native Windowing implementations.
* [include/vsg/viewer/Viewer.h](Viewer.h) - high level viewer class for managing vsg::Window(s) and rendering of graphics to them.
