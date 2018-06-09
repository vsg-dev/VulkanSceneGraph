#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>

int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    osgViewer::Viewer viewer(arguments);

    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );

    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);
    if (!loadedModel)
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    viewer.setSceneData(loadedModel.get());

    viewer.realize();

    while(!viewer.done())
    {
        viewer.advance();
        viewer.eventTraversal();
        viewer.updateTraversal();
        viewer.renderingTraversals();
    }

    return 0;
}