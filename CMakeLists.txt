cmake_minimum_required(VERSION 3.26)
project(py VERSION 0.1.0 LANGUAGES C CXX)

include(CMakePrintHelpers)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH 1)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development NumPy)
if(VERBOSE)
    cmake_print_variables(Python3_INCLUDE_DIRS)
    cmake_print_variables(Python3_NumPy_INCLUDE_DIRS)
endif()

# TODO: make a FindPd.cmake find module to address the following PD_* items:

execute_process(
    COMMAND "${Python3_EXECUTABLE}" -c "import platform; print('.pd_' + platform.system().lower())"
    OUTPUT_VARIABLE PD_EXTERNAL_SUFFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(VERBOSE)
    cmake_print_variables(PD_EXTERNAL_SUFFIX)
endif()

set(PD_EXTERNALS_PATH "~/Documents/Pd/externals" CACHE PATH "Pd externals path")

if(UNIX AND NOT APPLE)
    set(PD_SOURCE_PATH_DEFAULT /usr/local/pd/src)
elseif(UNIX AND APPLE)
    set(PD_SOURCE_PATH_DEFAULT /Applications/Pd-0.54-0.app/Contents/Resources/src)
elseif(WIN32)
    set(PD_SOURCE_PATH_DEFAULT c:/pd/src)
endif()
set(PD_SOURCE_PATH ${PD_SOURCE_PATH_DEFAULT} CACHE PATH "Pd src path")

set(STRIP_TARGET OFF CACHE BOOL "Strip unneeded symbols from compiled library (saves some disk space)")

set(FLEXT_SOURCE_PATH /usr/local/include/flext CACHE PATH "Flext source path")
set(FLEXT_BUILD_TYPE SINGLE CACHE STRING "Flext build type")
set_property(CACHE FLEXT_BUILD_TYPE PROPERTY STRINGS "SINGLE" "SHARED" "MULTI")

add_library(objects OBJECT
    source/bound.cpp
    source/clmeth.cpp
    source/main.cpp
    source/modmeth.cpp
    source/py.cpp
    source/pyargs.cpp
    source/pyatom.cpp
    source/pybase.cpp
    source/pybuffer.cpp
    source/pybundle.cpp
    source/pycompat.cpp
    source/pydsp.cpp
    source/pyext.cpp
    source/pymeth.cpp
    source/pysymbol.cpp
    source/register.cpp)

target_compile_definitions(objects PRIVATE PD)
target_compile_definitions(objects PRIVATE FLEXT_INLINE)
target_compile_definitions(objects PRIVATE FLEXT_ATTRIBUTES=1)
if(FLEXT_BUILD_TYPE STREQUAL MULTI)
    target_compile_definitions(objects PRIVATE FLEXT_THREADS)
elseif(FLEXT_BUILD_TYPE STREQUAL SHARED)
    target_compile_definitions(objects PRIVATE FLEXT_SHARED)
endif()
if(UNIX)
    target_compile_definitions(objects PRIVATE UNIX)
endif()
target_compile_options(objects PRIVATE -O3 -Wl,--trace -fPIC -fcheck-new -Wall -Wextra -Wshadow -Winline -Wstrict-aliasing -ffast-math -funroll-loops -fomit-frame-pointer)
target_include_directories(objects PRIVATE ${PD_SOURCE_PATH})
target_include_directories(objects PRIVATE ${FLEXT_SOURCE_PATH})
target_link_libraries(objects PRIVATE Python3::Python Python3::NumPy)

# ---------------------------------------------- #
# BUILDING PD EXTERNAL PY/PYEXT

add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:objects>)
target_compile_options(${PROJECT_NAME} PRIVATE -rdynamic -shared -fPIC -Wl,-rpath,"\$ORIGIN",--enable-new-dtags)
target_link_libraries(${PROJECT_NAME}
    PRIVATE -lc
    PRIVATE -lm
    PRIVATE -lstdc++
)
target_link_libraries(${PROJECT_NAME} PRIVATE Python3::Python Python3::NumPy)
if(FLEXT_BUILD_TYPE STREQUAL SHARED)
    target_compile_options(${PROJECT_NAME} PRIVATE lflext-pd)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX "${PD_EXTERNAL_SUFFIX}")

if(STRIP_TARGET)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND strip --strip-unneeded ${CMAKE_BINARY_DIR}/${PROJECT_NAME}${PD_EXTERNAL_SUFFIX}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Stripping file..."
    )
endif()

# install to Pd externals directory (run `cmake --install <build-dir>` to install)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}${PD_EXTERNAL_SUFFIX} DESTINATION ${PD_EXTERNALS_PATH}/${PROJECT_NAME})
