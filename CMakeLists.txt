cmake_minimum_required(VERSION 3.30)
project(rtree)

set(CMAKE_CXX_STANDARD 20)

add_executable(rtree src/main.cpp
        src/geometry/Point3D.h
        src/rtree/MBR.h
        src/rtree/RTreeNode.h
        src/rtree/RTree3D.h
        src/rtree/RTreeInnerNode.h
        src/rtree/RTree3D.h
        src/geometry/Triangle3D.h
        src/rtree/RTree3D.cpp
        src/rtree/MBR.cpp)
