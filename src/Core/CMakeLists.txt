set( SRCS "G2AssimpScene.cpp" "G2AssimpSceneTextureGeneration.cpp")
set( INCS "G2AssimpScene.h")

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${OPENCL_INCLUDE_DIRS} ${CGAL_INCLUDE_DIRS})

add_library(Core ${SRCS} ${INCS})
source_group(headers FILES ${INCS})
