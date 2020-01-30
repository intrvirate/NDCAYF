#https://www.glfw.org/docs/latest/build_guide.html#build_link_cmake_package

cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OpenGL_GL_PREFERENCE GLVND) #GLVND

project(NDCAYF LANGUAGES CXX)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

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
)

target_link_libraries(NDCAYF glfw)

target_include_directories(NDCAYF PUBLIC ${OPENGL_INCLUDE_DIRS})
target_link_libraries(NDCAYF ${OPENGL_LIBRARIES})

target_include_directories(NDCAYF PUBLIC ${GLEW_INCLUDE_DIRS})
target_link_libraries(NDCAYF ${GLEW_LIBRARIES})

target_include_directories(NDCAYF PRIVATE
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/util"
                          )
