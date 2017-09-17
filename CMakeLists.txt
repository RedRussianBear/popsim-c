cmake_minimum_required(VERSION 3.3)
project(popsim)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread -fpermissive")

find_package (OpenGL REQUIRED)
find_package (GLUT REQUIRED)


include_directories(${OPENGL_INCLUDE_DIR}  ${GLUT_INCLUDE_DIRS})

file(GLOB SOURCE_FILES
        *.c
        *.h
        SOIL/*.c
        SOIL/*.h
        )

add_executable(main.c ${SOURCE_FILES})

target_link_libraries (main.c ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} )