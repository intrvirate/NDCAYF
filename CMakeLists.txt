#https://www.glfw.org/docs/latest/build_guide.html#build_link_cmake_package

cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OpenGL_GL_PREFERENCE GLVND) #GLVND

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

project(NDCAYF LANGUAGES CXX)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(assimp REQUIRED)
find_package(Bullet REQUIRED)

find_package(OpenAL REQUIRED)

add_custom_target(run
    COMMAND ${PROJECT_BINARY_DIR}/${PROJECT_NAME}
    USES_TERMINAL
    )


set(SOURCES
    main.cpp
    util/browser/Browser.cpp
    util/bulletDebug/collisiondebugdrawer.cpp
    util/editor/editor.cpp
    util/globalStateHandlers.cpp
    util/groundGridGeneration.cpp
    util/handleinput.cpp
    util/loadMenu.cpp
    util/loadMenu.hpp
    util/loadShaders.cpp
    util/networking/MusicPlayer/stream.cpp
    util/networking/TCP/FileGet.cpp
    util/networking/TCP/FileUpload.cpp
    util/networking/TCP/MusicStreamer.cpp
    util/networking/TCP/TCP.cpp
    util/networking/client.cpp
    util/networking/getLan.cpp
    util/object/object.cpp
    util/object/object_gl.cpp
    util/otherhandlers.cpp
    util/render/render2D.cpp
    util/render/render3D.cpp
    util/render/skybox.cpp
    util/render/startupConsole.cpp
    )

set(IMGUI_SOURCES
    util/imgui/imgui.cpp
    util/imgui/imgui_widgets.cpp
    util/imgui/imgui_draw.cpp
    util/imgui/imgui_impl_glfw.cpp
    util/imgui/imgui_impl_opengl3.cpp
    )


add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES})

target_link_libraries(${PROJECT_NAME} glfw)

target_include_directories(${PROJECT_NAME} PUBLIC ${ASSIMP_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${ASSIMP_LIBRARIES})

target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

target_include_directories(${PROJECT_NAME} PUBLIC ${GLEW_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES})

target_include_directories(${PROJECT_NAME} PUBLIC ${BULLET_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${BULLET_LIBRARIES})


target_include_directories(NDCAYF PRIVATE
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/util"
    "${CMAKE_SOURCE_DIR}/util/browser"
    "${CMAKE_SOURCE_DIR}/util/bulletDebug"
    "${CMAKE_SOURCE_DIR}/util/editor"
    "${CMAKE_SOURCE_DIR}/util/imgui"
    "${CMAKE_SOURCE_DIR}/util/networking"
    "${CMAKE_SOURCE_DIR}/util/object"
    "${CMAKE_SOURCE_DIR}/util/render"
    "${CMAKE_SOURCE_DIR}"
    )


target_include_directories(NDCAYF PUBLIC ${OpenAL_INCLUDE_DIR})
target_link_libraries(NDCAYF ${OPENAL_LIBRARY})
