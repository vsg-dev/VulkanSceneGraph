INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/include
)

SET(HEADERS
    observer_ptr.h
    ref_ptr.h

    Auxiliary.h
    Group.h
    Node.h
    Object.h
    Visitor.h
)

SET(SOURCES
    Auxiliary.cpp
    Group.cpp
    Node.cpp
    Object.cpp
    Visitor.cpp
)

ADD_LIBRARY(vsg SHARED ${HEADER_FILES} ${SOURCES})
