INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/include
    ${OSG_INCLUDE_DIR}
    ${Vulkan_INCLUDE_DIR}
)

SET(HEADERS
    ImageUtils.h
)

SET(SOURCES
    ImageUtils.cpp
)

if   (VSG_SHARED_LIBRARY)
    add_definitions(-DVSG_LIBRARY)
    add_library(osg2vsg SHARED ${HEADER_FILES} ${SOURCES})
else ()
    add_definitions(-DVSG_LIBRARY_STATIC)
    add_library(osg2vsg STATIC ${HEADER_FILES} ${SOURCES})
endif()

TARGET_LINK_LIBRARIES(osg2vsg ${Vulkan_LIBRARY} vsg ${OSG_LIBRARIES} ${OSGUTIL_LIBRARIES} ${OSGDB_LIBRARIES})
