# include/vsg/viewer headers

The include/vsg/viewer header directory contains the Windowing  and Viewer classes required to create a Vulkan window(s) and viewer to render to it/then.

* [include/vsg/viewer/Window.h](Window.h) - base class for creation of Windows with Vulkan support.  Subclasses from vsg::Window provide the implementation for the different target platforms, these can be found in include/vsg/platform directory
* [include/vsg/viewer/Viewer.h](Viewer.h) - high level viewer class for managing vsg::Window(s) and rendering graphics to them.
