list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

find_package(Eigen3 REQUIRED) 
find_package(OpenGL REQUIRED)
find_package(OpenCL REQUIRED)
find_package(CGAL REQUIRED)
find_package(FIELD3D REQUIRED)
message (STATUS CGAL FOUND: ${CGAL_FOUND})
