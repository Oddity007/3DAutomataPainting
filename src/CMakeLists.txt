cmake_minimum_required(VERSION 2.8)
project(gx)

include(cmake/CMakeLists.txt)

set( CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/.. )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(stb)

add_subdirectory(assimp)
include_directories(assimp/include)

add_subdirectory(glfw-3.1.1)
include_directories(glfw-3.1.1/include)

#set(CGAL_DISABLE_GMP ON)
#add_subdirectory(CGAL)
#include_directories(CGAL/include)

include_directories(
	${PROJECT_SOURCE_DIR}
	${GLFW_INCLUDE_DIRS}
	${EIGEN3_INCLUDE_DIR}
	${CGAL_INCLUDE_DIR}
	${FIELD3D_INCLUDE_DIRS}
)

#target_link_libraries(p4 glfw ${GLFW_LIBRARIES} assimp ${BULLET_LIBRARIES})

add_subdirectory(Core)

add_subdirectory(lua)

# a library of rendering code
add_subdirectory(Renderer)

# the main application
add_subdirectory(application)
