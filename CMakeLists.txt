#cmake_minimum_required(VERSION 3.1)
# should be fine with any version that supports findGLUT. I think this one is specific to that
cmake_minimum_required(VERSION 3.22)
project(assign3)

OPTION(EXTRAS "Due to the difference between old OpenGL and new OpenGL (and my potential reluctance to fully rewrite the code!) the new stuff is disabled by default!" OFF)

configure_file(include/config.h.in include/config.h @ONLY)

if(${CMAKE_BUILD_TYPE} MATCHES Debug AND LINUX)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

# Lab computers are fine with CPP20 but to be on the safe side use 17.
# I run with GCC
set(CMAKE_CXX_STANDARD 17)

# I like absolute directories since I think relative file paths are ugly and hard to read
include_directories(include)
include_directories(${CMAKE_BINARY_DIR}/include/)
#file(GLOB_RECURSE SRC_FILES "src/*.cpp")

# don't try to build other mode's files as we will use the same names of functions to make it easier
# we have to do this because GL
if (${EXTRAS})
    file(GLOB SRC_FILES "src/*.cpp" "src/high_perf/*.cpp")
else ()
    file(GLOB SRC_FILES "src/*.cpp" "src/basic/*.cpp")
endif ()

# Include my utility library
#if(WIN32)
    #include_directories(libs/BLT/include)
#else()
    add_subdirectory(libs/BLT)
#endif()

#set(OpenGL_GL_PREFERENCE LEGACY)

if(UNIX)
    find_package(GLUT)
else()
    if(NOT CMAKE_CL_64)
        message("Using 32bit")
        set(GLUT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libs/freeglut/include/")
        set(GLUT_LIBRARIES "${CMAKE_SOURCE_DIR}/libs/freeglut/libs/win32/freeglut.lib")
        file(COPY "${CMAKE_SOURCE_DIR}/freeglut.dll" DESTINATION "${CMAKE_BINARY_DIR}/")
        #set(GLUT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libs/freeglut_old/include/")
        #set(GLUT_LIBRARIES "${CMAKE_SOURCE_DIR}/libs/freeglut_old/include/GL/freeglut.lib")
        #file(COPY "${CMAKE_SOURCE_DIR}/freeglut_ross_old.dll" DESTINATION "${CMAKE_BINARY_DIR}/")
        #file(RENAME "${CMAKE_BINARY_DIR}/freeglut_ross_old.dll" "${CMAKE_BINARY_DIR}/freeglut_ross_old.dll")
    else()
        message("Using 64bit")
        set(GLUT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libs/freeglut/include/")
        set(GLUT_LIBRARIES "${CMAKE_SOURCE_DIR}/libs/freeglut/libs/win64/freeglut.lib")
        file(COPY "${CMAKE_SOURCE_DIR}/freeglut64.dll" DESTINATION "${CMAKE_BINARY_DIR}/")
        file(COPY "${CMAKE_SOURCE_DIR}/freeglut64.dll" DESTINATION "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/")
        file(RENAME "${CMAKE_BINARY_DIR}/freeglut64.dll" "${CMAKE_BINARY_DIR}/freeglut.dll")
    endif()
endif()

file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR}/)

find_package(OpenGL)

include_directories(${GLUT_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIRS})

message("${GLUT_INCLUDE_DIR} && ${GLUT_INCLUDE_DIRS} && ${GLUT_LIBRARIES}")

add_executable(assign3 ${SRC_FILES})

#target_link_libraries(assign1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libraries/glut/lib/freeglut.dll)
#target_link_libraries(assign3 PRIVATE ${CMAKE_SOURCE_DIR}/libs/freeglut/libs/win32/freeglut.lib)
target_link_libraries(assign3 PRIVATE ${GLUT_LIBRARIES})
target_link_libraries(assign3 PRIVATE OpenGL::GL)
target_link_libraries(assign3 PRIVATE OpenGL::GLU)
target_link_libraries(assign3 PRIVATE BLT)
if(UNIX)
	target_link_libraries(assign3 PRIVATE OpenGL::GLX)
else()
    #target_link_libraries(assign3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/BLT.lib)
endif()

if(MSVC)
    #target_compile_options(assign3 PRIVATE /W4)
else()
    target_compile_options(assign3 PRIVATE -Wall -Wextra -Wpedantic)
endif()
