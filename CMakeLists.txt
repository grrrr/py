cmake_minimum_required(VERSION 3.0.0)
project(py VERSION 0.1.0 LANGUAGES C CXX)


set(CMAKE_INSTALL_RPATH_USE_LINK_PATH 1)

# path to pythonX.X.so (e.g python3.10.so)
if(NOT PYTHON_SHARED_LIB_PATH)
    set(PYTHON_SHARED_LIB_PATH "/usr/lib64")
endif()

# path search by puredata for externals (preferences>edit preferences...>paths in new pd versions)
if(NOT PATH_TO_PD_EXTERNALS)
    set(PATH_TO_PD_EXTERNALS "~/Documents/Pd/externals")
endif()

# path to libpd source files 
if(NOT PD_SOURCE_PATH)
    set(PD_SOURCE_PATH /usr/local/pd/src)
endif()

# path to flext source files ; default is assumed py/pyext and flext repos have been cloned in a common parent directory
if(NOT FLEXT_SOURCE_PATH)
    set(FLEXT_SOURCE_PATH ../flext/source)
endif()

# path to python source files (can be found by running `sudo find / -iname "Python.h"`)
if(NOT PYTHON_SOURCE_PATH)
    set(PYTHON_SOURCE_PATH /usr/include/python3.10)
endif()

# your python version
if(NOT PYTHON_VERSION)
    set(PYTHON_VERSION 3.10)
endif()

# flext build
if(NOT FLEXT_BUILD_TYPE)
    set(FLEXT_BUILD_TYPE SINGLE)
endif()

include(CMakePrintHelpers)
cmake_print_variables(PYTHON_SOURCE_PATH)
set(CMAKE_INSTALL_RPATH ${PYTHON_SHARED_LIB_PATH})
set(CMAKE_BUILD_RPATH ${PYTHON_SHARED_LIB_PATH})
# ---------------------------------------------- #
# BUILDING OBJECT FILES

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

target_compile_options(objects PRIVATE -DPD -DUNIX -O3 -Wl,--trace -fPIC -fcheck-new -DFLEXT_INLINE -DFLEXT_ATTRIBUTES=1 -Wall -Wextra -Wshadow -Winline -Wstrict-aliasing -ffast-math -funroll-loops -fomit-frame-pointer -march=core2 -mfpmath=sse -msse -msse2 -msse3)

# add flext flag depending on chosen build type
if(FLEXT_BUILD_TYPE STREQUAL MULTI)
    message("setting threads")
    target_compile_options(objects PRIVATE -DFLEXT_THREADS)
elseif(FLEXT_BUILD_TYPE STREQUAL SHARED) 
    target_compile_options(objects PRIVATE -DFLEXT_SHARED)
endif()

target_include_directories(objects PRIVATE
    ${PD_SOURCE_PATH}
    ${FLEXT_SOURCE_PATH}
    ${PYTHON_SOURCE_PATH})

target_link_libraries(objects PRIVATE -lpython${PYTHON_VERSION})

# ---------------------------------------------- #
# BUILDING PD EXTERNAL PY/PYEXT

add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:objects>)

target_compile_options(${PROJECT_NAME} PRIVATE -rdynamic -shared -fPIC -Wl,-rpath,"\$ORIGIN",--enable-new-dtags)

target_link_directories(${PROJECT_NAME} PRIVATE /usr/local/lib)
target_link_directories(${PROJECT_NAME} PRIVATE ${PYTHON_SHARED_LIB_PATH})

target_link_libraries(${PROJECT_NAME}
    PRIVATE -lc 
    PRIVATE -lm 
    PRIVATE -lstdc++
    PRIVATE -lpython${PYTHON_VERSION})

if(FLEXT_BUILD_TYPE STREQUAL SHARED) 
    target_compile_options(${PROJECT_NAME} PRIVATE lflext-pd)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".pd_linux")

# stripping file to make it lighter 
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
COMMAND strip --strip-unneeded ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pd_linux
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Stripping file..."
)

# copying file to Pd externals directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
COMMAND mkdir -p ${PATH_TO_PD_EXTERNALS}/${PROJECT_NAME} && cp ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pd_linux ${PATH_TO_PD_EXTERNALS}/${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Copying file to Pd externals directory..."
)