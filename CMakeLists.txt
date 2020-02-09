#https://www.glfw.org/docs/latest/build_guide.html#build_link_cmake_package

cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OpenGL_GL_PREFERENCE GLVND) #GLVND

project(NDCAYF LANGUAGES CXX)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(assimp REQUIRED)

add_executable(NDCAYF
                main.cpp
                util/json.hpp
                util/loadMenu.cpp
                util/loadMenu.hpp
                util/loadShaders.hpp
                util/loadShaders.cpp
                util/handleinput.hpp
                util/handleinput.cpp
                util/otherhandlers.hpp
                util/otherhandlers.cpp
                util/vertexGrid.hpp
                util/render/render3D.hpp
                util/render/render3D.cpp
                util/render/render2D.hpp
                util/render/render2D.cpp
                util/stb_image.h
                util/globalStateHandlers.hpp
                util/globalStateHandlers.cpp
                #util/object/model.cpp
                util/object/model.hpp
                #util/object/mesh.cpp
                util/object/mesh.hpp
                #util/object/shader.cpp
                util/object/shader.hpp
                util/groundGridGeneration.cpp
                util/groundGridGeneration.hpp
)

target_link_libraries(NDCAYF glfw)

target_include_directories(NDCAYF PUBLIC ${ASSIMP_INCLUDE_DIRS})
target_link_libraries(NDCAYF ${ASSIMP_LIBRARIES})

target_include_directories(NDCAYF PUBLIC ${OPENGL_INCLUDE_DIRS})
target_link_libraries(NDCAYF ${OPENGL_LIBRARIES})

target_include_directories(NDCAYF PUBLIC ${GLEW_INCLUDE_DIRS})
target_link_libraries(NDCAYF ${GLEW_LIBRARIES})

target_include_directories(NDCAYF PRIVATE
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/util"
                          )
