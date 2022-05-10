#include <vsg/platform/ios/iOS_ViewController.h>
#include <vsg/platform/ios/iOS_Window.h>
#include <vsg/viewer/Viewer.h>

#pragma mark -
#pragma mark vsg_iOS_ViewController

@implementation vsg_iOS_ViewController {
    CADisplayLink* _displayLink;
    vsg::ref_ptr<vsg::WindowTraits>     _traits;
    vsg::ref_ptr<vsg::Viewer>           _vsgViewer;
}

- (instancetype)initWithTraits:(vsg::ref_ptr<vsg::WindowTraits>)traits andVsgViewer:(vsg::ref_ptr<vsg::Viewer>) vsgViewer
{
    self = [super init];
    _traits = traits;
    _vsgViewer = vsgViewer;
    return self;
}

- (void)loadView
{
    // We need to create a vsgView because doing so we ensure it has a CAMetalLayer
    // otherwise it will make a view with a CALayer which is not compatible with MoltenVK.
    // Also, very important to give it a size or it won't let access to any CAMetalDrawable
    // which will cause a massive hang in the device forcing it to be rebooted in order
    // to regain control of it.
    CGRect frame;
    frame.origin.x = _traits->x;
    frame.origin.y = _traits->y;
    frame.size.width  = _traits->width <= 0 ? 1 : _traits->width;
    frame.size.height = _traits->height <= 0 ? 1 : _traits->height;
    vsg_iOS_View* view = [[vsg_iOS_View alloc] initWithFrame:frame];
    self.view = view;
}

-(void) dealloc {
    _traits = nullptr;
    _vsgViewer = nullptr;
    _vsgWindow = nullptr;
}


-(void) viewDidLoad {
    [super viewDidLoad];
    
    self.view.contentScaleFactor = UIScreen.mainScreen.nativeScale;
    uint32_t fps = 60;
    _displayLink = [CADisplayLink displayLinkWithTarget: self selector: @selector(renderLoop)];
    [_displayLink setPreferredFramesPerSecond: fps];
    [_displayLink addToRunLoop: NSRunLoop.currentRunLoop forMode: NSDefaultRunLoopMode];
}

-(void) renderLoop {
   if (self->_vsgViewer->advanceToNextFrame())
   {
       self->_vsgViewer->compile();
       self->_vsgViewer->handleEvents();
       self->_vsgViewer->update();
       self->_vsgViewer->recordAndSubmit();
       self->_vsgViewer->present();
   }
}

// Allow device rotation to resize the swapchain
-(void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id)coordinator {
    [super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
    // TODO implement this
}

@end


#pragma mark -
#pragma mark vsg_iOS_View

@implementation vsg_iOS_View

/** Returns a Metal-compatible layer (required by Vulkan/MoltenVK. */
+(Class) layerClass { return [CAMetalLayer class]; }

@end
