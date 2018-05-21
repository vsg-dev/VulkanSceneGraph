cmake_minimum_required(VERSION 3.5)

PROJECT(VulkanPlayground)

FIND_PACKAGE(OpenThreads)
FIND_PACKAGE(osg)
FIND_PACKAGE(osgDB)
FIND_PACKAGE(osgUtil)
FIND_PACKAGE(osgGA)
FIND_PACKAGE(osgViewer)

add_subdirectory(src)