INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/include
)

SET(HEADERS

    core/observer_ptr.h
    core/ref_ptr.h
    core/Auxiliary.h
    core/Object.h
    core/Value.h
    core/Visitor.h

    nodes/Group.h
    nodes/Node.h
    nodes/QuadGroup.h
    nodes/LOD.h
)

SET(SOURCES
    core/Auxiliary.cpp
    core/Object.cpp
    core/Visitor.cpp

    nodes/Group.cpp
    nodes/Node.cpp
)

ADD_LIBRARY(vsg SHARED ${HEADER_FILES} ${SOURCES})
