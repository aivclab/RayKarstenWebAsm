project(RayKarsten)
cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 11)

set (SRC
    src/RayLib.cpp 
    src/RayLib.h
    src/EmWrapper.cpp
)

set (APPSRC
    src/RayLib.cpp 
    src/RayLib.h
    src/main.cpp
)

if (CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    add_executable(${PROJECT_NAME} ${SRC})
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11 )
    #set(CMAKE_EXE_LINKER_FLAGS "-O3 -s EXPORTED_FUNCTIONS=['_rayCastImage'] -s SIDE_MODULE=1 -o rayKarsten.wasm" )
    set(CMAKE_EXE_LINKER_FLAGS "--bind -O3 -s MODULARIZE=1"  -s SIMD=1  )
    set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/src)
else()
    add_executable(${PROJECT_NAME} ${APPSRC})
    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11 )
    SET(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()



