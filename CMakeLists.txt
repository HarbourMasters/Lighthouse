cmake_minimum_required(VERSION 3.10)
project(Lighthouse LANGUAGES C CXX)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_VERBOSE_MAKEFILE OFF)

# Option to select build
option(LIGHTHOUSE_P "Build the LIGHTHOUSE" ON)

# Add tools needed for the build
add_subdirectory(libultraship)

# Set options for Torch before adding it as a subdirectory
set(USE_STANDALONE OFF CACHE BOOL "Build Torch in standalone mode" FORCE)
set(BUILD_STORMLIB OFF CACHE BOOL "Build StormLib for Torch" FORCE)
set(BUILD_SM64 OFF CACHE BOOL "Build with Super Mario 64 support" FORCE)
set(BUILD_MK64 OFF CACHE BOOL "Build with Mario Kart 64 support" FORCE)
set(BUILD_SF64 OFF CACHE BOOL "Build with Star Fox 64 support" FORCE)
set(BUILD_FZERO OFF CACHE BOOL "Build with F-Zero X support" FORCE)
add_subdirectory(tools/Torch)

# Glob include files from the include directory (without filtering)
file(GLOB_RECURSE ALL_INCLUDES "${PROJECT_SOURCE_DIR}/include/**/*.c" "${PROJECT_SOURCE_DIR}/include/*.h" "${PROJECT_SOURCE_DIR}/include/**/*.h")

# Glob all .c and .h files from src and wip directories.
file(GLOB_RECURSE ALL_SRCS "${PROJECT_SOURCE_DIR}/src/**/*.c" "${PROJECT_SOURCE_DIR}/src/**/*.h")
file(GLOB_RECURSE ALL_WIPS "${PROJECT_SOURCE_DIR}/wip/**/*.cpp" "${PROJECT_SOURCE_DIR}/wip/**/*.c" "${PROJECT_SOURCE_DIR}/wip/**/*.h")

# Define directories to exclude (only for src and wip)
set(EXCLUDE_DIRS
    "${PROJECT_SOURCE_DIR}/src/core1/audio"
    "${PROJECT_SOURCE_DIR}/src/core1/gu"
    "${PROJECT_SOURCE_DIR}/src/core1/io"
    "${PROJECT_SOURCE_DIR}/src/core1/os"
    "${PROJECT_SOURCE_DIR}/src/done"
    "${PROJECT_SOURCE_DIR}/include/2.0L"
    "${PROJECT_SOURCE_DIR}/wip/starship"
)

# Function to filter out files whose paths start with any exclude directory.
function(filter_excluded_files out_var)
    set(filtered_list "")
    foreach(file ${ARGN})
        set(skip FALSE)
        foreach(exclude ${EXCLUDE_DIRS})
            if(file MATCHES "^${exclude}(/|$)")
                set(skip TRUE)
                break()
            endif() # if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        endforeach()
        if(NOT skip)
            list(APPEND filtered_list ${file})
        endif()
    endforeach()
    set(${out_var} ${filtered_list} PARENT_SCOPE)
endfunction()

# Filter source files from src and wip only.
filter_excluded_files(FILTERED_INCLUDES ${ALL_INCLUDES})
filter_excluded_files(FILTERED_SRCS ${ALL_SRCS})
filter_excluded_files(FILTERED_WIPS ${ALL_WIPS})

# Combine the unfiltered include files with the filtered source files.
set(ALL_FILES ${FILTERED_INCLUDES} ${FILTERED_SRCS} ${FILTERED_WIPS})

# Optionally, print the list of filtered files (for debugging)
#message(STATUS "Final source files:")
#foreach(f ${ALL_FILES})
#    message(STATUS "  ${f}")
#endforeach()

# Create the executable for the LIGHTHOUSE build.
add_executable(${PROJECT_NAME} ${ALL_FILES})

# Check the compiler and set the appropriate flags
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(${PROJECT_NAME} PRIVATE -w)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE /W0)
endif()

# Gather all subdirectories recursively (for include directories)
file(GLOB_RECURSE ALL_INCLUDESUBDIRS LIST_DIRECTORIES true "${PROJECT_SOURCE_DIR}/include/*")
file(GLOB_RECURSE ALL_WIPSUBDIRS LIST_DIRECTORIES true "${PROJECT_SOURCE_DIR}/wip/*")

# Filter out the console code directories from the include directories (if needed)
set(FILTERED_INCLUDESUBDIRS "")
foreach(DIR ${ALL_INCLUDESUBDIRS})
    if (NOT DIR MATCHES "${PROJECT_SOURCE_DIR}/include/2.0L")
        list(APPEND FILTERED_INCLUDESUBDIRS ${DIR})
    endif()
endforeach()

# Set target-specific include directories.
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/libultraship/include
    ${PROJECT_SOURCE_DIR}/tools/Torch/src
    ${PROJECT_SOURCE_DIR}/wip/pc
    ${PROJECT_SOURCE_DIR}/wip/pc/custom_includes
    ${PROJECT_SOURCE_DIR}/wip/port
    ${PROJECT_SOURCE_DIR}/wip
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${ALL_WIPSUBDIRS}
    ${FILTERED_INCLUDESUBDIRS}
)

# Add additional source files explicitly if needed.
target_sources(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/wip/pc/compat.h
    ${PROJECT_SOURCE_DIR}/wip/pc/custom_includes/ultra64.h
    ${PROJECT_SOURCE_DIR}/wip/pc/custom_includes/ultratypes.h
    ${PROJECT_SOURCE_DIR}/wip/pc/main.cpp
)

add_dependencies(${PROJECT_NAME} libultraship)

target_compile_definitions(${PROJECT_NAME} PRIVATE LIGHTHOUSE_P _LANGUAGE_C F3DEX_GBI)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
if (CMAKE_SYSTEM_NAME MATCHES "Darwin|iOS|NintendoSwitch|CafeOS")
    if (CMAKE_SYSTEM_NAME MATCHES "Darwin|iOS")
        target_compile_options(${PROJECT_NAME} PRIVATE
            -Wall -Wextra -Wno-error
            -Wno-return-type
            -Wno-unused-parameter
            -Wno-unused-function
            -Wno-unused-variable
            -Wno-missing-field-initializers
            -Wno-parentheses
            -Wno-narrowing
            -Wno-missing-braces
            -Wno-int-conversion
            $<$<COMPILE_LANGUAGE:C>:
                -Werror-implicit-function-declaration
                -Wno-incompatible-pointer-types
                -fpermissive
            >
            $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>
            $<$<COMPILE_LANGUAGE:CXX>:
                -Wno-c++11-narrowing
                -Wno-deprecated-enum-enum-conversion
            >
            -pthread
        )

        target_link_options(${PROJECT_NAME} PRIVATE
            -pthread
        )
    elseif (CMAKE_SYSTEM_NAME STREQUAL "NintendoSwitch")
        target_compile_options(${PROJECT_NAME} PRIVATE
            -Wall -Wextra -Wno-error
            -Wno-return-type
            -Wno-unused-parameter
            -Wno-unused-function
            -Wno-unused-variable
            -Wno-missing-field-initializers
            -Wno-parentheses
            -Wno-narrowing
            -Wno-missing-braces
            -Wno-int-conversion
            $<$<COMPILE_LANGUAGE:C>:
                -Werror-implicit-function-declaration
                -Wno-incompatible-pointer-types
                -fpermissive
            >
            $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>
            $<$<COMPILE_LANGUAGE:CXX>:
                -Wno-c++11-narrowing
                -Wno-deprecated-enum-enum-conversion
            >
            -pthread
        )

        target_link_options(${PROJECT_NAME} PRIVATE
            -pthread
        )
    elseif (CMAKE_SYSTEM_NAME STREQUAL "CafeOS")
        target_compile_options(${PROJECT_NAME} PRIVATE
            -O2

            # disable some warnings to not clutter output
            -Wno-multichar
            -Wno-return-type
            -Wno-narrowing
            -Wno-switch-outside-range
            $<$<COMPILE_LANGUAGE:C>:
                -Werror-implicit-function-declaration
                -Wno-incompatible-pointer-types
                -Wno-discarded-array-qualifiers
                -Wno-discarded-qualifiers
                -Wno-int-conversion
                -Wno-builtin-declaration-mismatch
                -Wno-switch-unreachable
                -Wno-stringop-overflow
            >
        )
    endif()
  else()
      if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        set(CPU_OPTION -msse2 -mfpmath=sse)
      endif()

      target_compile_options(${PROJECT_NAME} PRIVATE
          -Wall -Wextra -Wno-error
          -Wno-unused-parameter
          -Wno-unused-function
          -Wno-unused-variable
          -Wno-missing-field-initializers
          -Wno-parentheses
          -Wno-narrowing
          -Wno-missing-braces
          $<$<COMPILE_LANGUAGE:C>:
              -Wno-int-conversion
              -Werror-implicit-function-declaration
              -Wno-incompatible-pointer-types
              -fpermissive
          >
          $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>
          $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-enum-enum-conversion>
          -pthread
    ${CPU_OPTION}
      )

      target_link_options(${PROJECT_NAME} PRIVATE
          -pthread
          -Wl,-export-dynamic
      )
  endif()
endif()

# Link the executable with needed libraries.
target_link_libraries(${PROJECT_NAME} PRIVATE libultraship torch)

