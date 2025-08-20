cmake_minimum_required(VERSION 3.13)

set(PROJ_NAME TinyReflection)
project(${PROJ_NAME} VERSION 0.9.0)

# ---- Include guards ----
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

#只有执行了project()后，PROJECT_SOURCE_DIR才有值
set(TINY_REFLECTION_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(TINY_REFLECTION_SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(TINY_REFLECTION_TEST_DIR ${PROJECT_SOURCE_DIR}/test)

file(GLOB_RECURSE HEAD_FILES "${TINY_REFLECTION_INCLUDE_DIR}/*.hpp" "${TINY_REFLECTION_INCLUDE_DIR}/*.h")
file(GLOB_RECURSE SRC_FILES "${TINY_REFLECTION_SRC_DIR}/*.cpp" "${TINY_REFLECTION_SRC_DIR}/*.c")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC_FILES} ${HEAD_FILES})
add_library(${PROJ_NAME} ${HEAD_FILES} ${SRC_FILES})
target_include_directories(${PROJ_NAME}	PUBLIC ${TINY_REFLECTION_INCLUDE_DIR})
target_compile_features(${PROJ_NAME} PRIVATE cxx_std_17)


option(TinyReflection_Test "TEST" ON)
if (TinyReflection_Test)
	add_subdirectory(test)
endif()