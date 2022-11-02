#include <UIKit/UIKit.h>


#include <vsg/app/WindowTraits.h>
#include <vsg/app/Window.h>

#pragma mark -
#pragma mark vsg_iOS_View
@interface vsg_iOS_View : UIView
@end


#pragma mark -
#pragma mark vsg_iOS_ViewController
@interface vsg_iOS_ViewController : UIViewController
    @property vsg::ref_ptr<vsg::Window> vsgWindow;
    - (instancetype)initWithTraits:(vsg::ref_ptr<vsg::WindowTraits>)traits andVsgViewer:(vsg::ref_ptr<vsg::Viewer>) vsgViewer;
@end

