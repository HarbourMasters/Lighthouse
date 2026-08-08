cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Set the project version and language
project(Lighthouse VERSION 1.0.2 LANGUAGES C CXX ASM)
include(FetchContent)

include(cmake/lighthouse-cvars.cmake)
include(cmake/lus-cvars.cmake)

if(APPLE)
  enable_language(OBJCXX)
endif()

set(EXCLUDE_MPQ_SUPPORT ON)

set(NATO_PHONETIC_ALPHABET
  "Alfa" "Bravo" "Charlie" "Delta" "Echo" "Foxtrot" "Golf" "Hotel"
  "India" "Juliett" "Kilo" "Lima" "Mike" "November" "Oscar" "Papa"
  "Quebec" "Romeo" "Sierra" "Tango" "Uniform" "Victor" "Whiskey"
  "Xray" "Yankee" "Zulu"
)

# Get the patch version number from the project version
math(EXPR PATCH_INDEX "${PROJECT_VERSION_PATCH}")

# Use the patch number to select the correct word
list(GET NATO_PHONETIC_ALPHABET ${PATCH_INDEX} PROJECT_PATCH_WORD)

set(PROJECT_BUILD_NAME "Split Rock ${PROJECT_PATCH_WORD}" CACHE STRING "" FORCE)
#set(PROJECT_TEAM "SonicDcer & Lywx" CACHE STRING "" FORCE)

# Get Git Information
execute_process(
    COMMAND git branch --show-current
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(CMAKE_PROJECT_GIT_BRANCH "${GIT_BRANCH}" CACHE STRING "Git branch" FORCE)

execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get only the first 7 characters of the hash
string(SUBSTRING "${GIT_COMMIT_HASH}" 0 7 SHORT_COMMIT_HASH)

set(CMAKE_PROJECT_GIT_COMMIT_HASH "${SHORT_COMMIT_HASH}" CACHE STRING "Git commit hash" FORCE)

execute_process(
    COMMAND git describe --tags --abbrev=0 --exact-match HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT GIT_COMMIT_TAG)
    set(GIT_COMMIT_TAG "" CACHE STRING "Git commit tag" FORCE)
endif()

set(CMAKE_PROJECT_GIT_COMMIT_TAG "${GIT_COMMIT_TAG}" CACHE STRING "Git commit tag" FORCE)

# Set the minimum version of CMake and the deployment target for macOS
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")

# Set the C++ standard and enable the MSVC parallel build option
set(CMAKE_CXX_STANDARD 20 CACHE STRING "The C++ standard to use")
set(CMAKE_C_STANDARD 11 CACHE STRING "The C standard to use")
option(ENABLE_ASAN "Enable AddressSanitizer (static)" OFF)
if(ENABLE_ASAN)
    if(MSVC AND NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
        add_compile_options(/fsanitize=address)
        # /fsanitize=address is incompatible with /ZI (Edit and Continue), downgrade to /Zi globally.
        string(REPLACE "/ZI" "/Zi" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
        string(REPLACE "/ZI" "/Zi" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        string(REPLACE "/ZI" "/Zi" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
        string(REPLACE "/ZI" "/Zi" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
        # Also disable /RTC which is incompatible with ASan
        string(REGEX REPLACE "/RTC[1su]+" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
        string(REGEX REPLACE "/RTC[1su]+" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        # Disable STL container annotations to avoid LNK2038 mismatches with pre-built vcpkg libs
        add_compile_definitions(_DISABLE_STRING_ANNOTATION _DISABLE_VECTOR_ANNOTATION)
    else()
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address -static-libsan)
    endif()
endif()

# Add a custom module path to locate additional CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
set(YAML_CPP_STATIC_DEFINE ON)
add_compile_definitions(YAML_CPP_STATIC_DEFINE)

if (WIN32)
include(cmake/automate-vcpkg.cmake)
set(VCPKG_TRIPLET x64-windows-static)
set(VCPKG_TARGET_TRIPLET x64-windows-static)
vcpkg_bootstrap()
vcpkg_install_packages(zlib bzip2 libzip libpng sdl2 glew glfw3 nlohmann-json tinyxml2 spdlog libogg libvorbis)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/config.yml" "${CMAKE_BINARY_DIR}/config.yml")
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CMAKE_SOURCE_DIR}/assets/" "${CMAKE_BINARY_DIR}/assets/")


if (MSVC)
    set(CPP "${CMAKE_C_COMPILER}" "/EP")
else()
    set(CPP "${CMAKE_C_COMPILER}" "-E" "-P" "-Wno-trigraphs" "-x" "c")
endif()

if(IOS)
    set(PLATFORM "OS64")
    include(cmake/ios.toolchain.cmake)
    add_compile_definitions(
        PLATFORM_IOS=1
    )
    set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "YOUR_TEAM_ID")
    set(PROJECT_ID "dev.net64.game")
endif()

################################################################################
# Set target arch type if empty. Visual studio solution generator provides it.
################################################################################
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(NOT CMAKE_VS_PLATFORM_NAME)
        set(CMAKE_VS_PLATFORM_NAME "x64")
    endif()
    message("${CMAKE_VS_PLATFORM_NAME} architecture in use")

    if(NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64"
        OR "${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32"))
        message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} arch is not supported!")
    endif()
endif()

################################################################################
# Global configuration types
################################################################################
if (CMAKE_SYSTEM_NAME STREQUAL "NintendoSwitch")
set(CMAKE_C_FLAGS_DEBUG "-g -ffast-math -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -ffast-math -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O3 -ffast-math -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math -DNDEBUG")
else()
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_OBJCXX_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()

# Set game compilation version
set(VERSION us)
option(USE_NETWORKING "Enable networking features" ON)
set(SKIP_XCODE_VERSION_CHECK ON)

# Add compile definitions for the target
add_compile_definitions(
    VERSION_US=1
    ENABLE_RUMBLE=1
    F3DEX_GBI=1
    _LANGUAGE_C
    _USE_MATH_DEFINES
    CIMGUI_DEFINE_ENUMS_AND_STRUCTS
    NON_MATCHING=1
    NON_EQUIVALENT=1
	AVOID_UB=1
    GBI_FLOATS=1
    N_MICRO=1
)

# Find necessary libraries
if (UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
endif()

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/src/port/build.c.in ${CMAKE_CURRENT_SOURCE_DIR}/src/port/build.c @ONLY)
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/properties.h.in ${CMAKE_CURRENT_SOURCE_DIR}/properties.h @ONLY)

set(Header_Files "resource.h")
source_group("include" FILES ${Header_Files})

# Flag enum name tables for the Flag Tracer, derived from include/enums.h.
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateFlagNames.cmake)

# Include directories
include_directories(
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/lib/ultralib/include"
    "${PROJECT_SOURCE_DIR}/src"
    "${CMAKE_BINARY_DIR}/generated"
)

# Collect source files to build the executable
message(${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE ALL_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    # "src/assets/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/port/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/ultralib/**/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/ultralib/**/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/**/*"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*"
)

list(FILTER ALL_FILES EXCLUDE REGEX "src/unused/")

list(APPEND ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Resource.rc)


# Create source groups that match the real file directory paths
foreach(source IN LISTS ALL_FILES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_adjusted "${source_path}")
    source_group("${source_path_adjusted}" FILES "${source}")
endforeach()

if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(IOS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/ios)

    set(STORYBOARD_FILE ${IOS_DIR}/Launch.storyboard)
    set(IMAGE_FILES ${IOS_DIR}/PoweredBy.png)
    set(ICON_FILES ${IOS_DIR}/Icon.png)

    list(APPEND ALL_FILES ${STORYBOARD_FILE} ${IMAGE_FILES} ${ICON_FILES})

    add_executable(${PROJECT_NAME} ${ALL_FILES})
    set_xcode_property(${PROJECT_NAME} PRODUCT_BUNDLE_IDENTIFIER ${PROJECT_ID} All)
    set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${IOS_DIR}/plist.in
        RESOURCE "${IMAGE_FILES};${STORYBOARD_FILE};${ICON_FILES}"
    )
else()
    add_executable(${PROJECT_NAME} ${ALL_FILES})
endif()

################################################################################
# MSVC runtime library
################################################################################
if (MSVC)
  if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	  get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
	  if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
		  string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
			  $<$<CONFIG:Debug>:
                MultiThreadedDebug
			  >
			  $<$<CONFIG:Release>:
                MultiThreaded
			  >
			  $<$<CONFIG:RelWithDebInfo>:
                MultiThreaded
			  >
			  $<$<CONFIG:MinSizeRel>:
                MultiThreaded
			  >
			  $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
		  )
	  elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
		  string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
			  $<$<CONFIG:Debug>:
				  MultiThreadedDebug
			  >
			  $<$<CONFIG:Release>:
				  MultiThreaded
			  >
			  $<$<CONFIG:RelWithDebInfo>:
				  MultiThreaded
			  >
			  $<$<CONFIG:MinSizeRel>:
				  MultiThreaded
			  >
			  $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
		  )
	  endif()
	  set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})
  endif()
endif()

#=================== SSE2NEON ===================
set(SSE2NEON_DIR ${CMAKE_BINARY_DIR}/_deps/sse2neon)
file(DOWNLOAD "https://raw.githubusercontent.com/DLTcollab/sse2neon/refs/heads/master/sse2neon.h" "${SSE2NEON_DIR}/sse2neon.h")

include_directories(${SSE2NEON_DIR})

FetchContent_Declare(
    dr_libs
    GIT_REPOSITORY https://github.com/mackron/dr_libs.git
	GIT_TAG da35f9d6c7374a95353fd1df1d394d44ab66cf01
)
FetchContent_MakeAvailable(dr_libs)

#==============================================================================#
# Libultraship Integration                                                     #
#==============================================================================#

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/include
  #${CMAKE_CURRENT_SOURCE_DIR}/include/2.0L
  #${CMAKE_CURRENT_SOURCE_DIR}/include/2.0L/PR
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/include/libultraship
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/src
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/src/log
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/src/debug
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/src/menu
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/src/utils
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/src/utils/binarytools
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/src/config
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/src/resource
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/src/resource/type
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/src/resource/factory
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/src/audio
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/src/window
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/src/window/gui
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/src/config
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/src/public
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/src/public/libultra
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/src/public/bridge
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/extern
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/extern/tinyxml2
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/libultraship/Lib/
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/libultraship/Lib/libjpeg/include/
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/libultraship/Lib/spdlog/include/
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/src/graphic/Fast3D/U64/PR
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/src/graphic
  #${CMAKE_CURRENT_SOURCE_DIR}/port
  ${SDL2_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${dr_libs_SOURCE_DIR}
)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}"
    )
    add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/config.yml ${CMAKE_BINARY_DIR}/config.yml
    )
endif()

add_subdirectory(libultraship ${CMAKE_CURRENT_SOURCE_DIR}/libultraship)
add_dependencies(${PROJECT_NAME} libultraship)
target_link_libraries(${PROJECT_NAME} PRIVATE libultraship)

# /fsanitize=address is incompatible with /ZI (Edit and Continue), downgrade to /Zi.
if(ENABLE_ASAN AND MSVC)
    foreach(_asan_target prism torch)
        if(TARGET ${_asan_target})
            get_target_property(_opts ${_asan_target} COMPILE_OPTIONS)
            if(_opts)
                list(TRANSFORM _opts REPLACE "^/ZI$" "/Zi")
                set_property(TARGET ${_asan_target} PROPERTY COMPILE_OPTIONS ${_opts})
            endif()
        endif()
    endforeach()
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG;"
            "_CRT_SECURE_NO_WARNINGS;"
            "ENABLE_DX11;"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "INCLUDE_GAME_PRINTF;"
        "UNICODE;"
        "_UNICODE;"
        "_CRT_SECURE_NO_WARNINGS;"
        "_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS;"
        "NOMINMAX;"
        "WIN32_LEAN_AND_MEAN;"
        "SPDLOG_ACTIVE_LEVEL=0;"
    )
    add_compile_definitions(STORMLIB_NO_AUTO_LINK)
	set(STORMLIB_NO_AUTO_LINK ON)
elseif (CMAKE_SYSTEM_NAME STREQUAL "CafeOS")
	target_compile_definitions(${PROJECT_NAME} PRIVATE
		"$<$<CONFIG:Debug>:"
			"_DEBUG"
		">"
		"$<$<CONFIG:Release>:"
			"NDEBUG"
		">"
        "SPDLOG_ACTIVE_LEVEL=3;"
		"SPDLOG_NO_THREAD_ID;"
        "SPDLOG_NO_TLS;"
		"STBI_NO_THREAD_LOCALS;"
	)
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU|Clang|AppleClang")
	target_compile_definitions(${PROJECT_NAME} PRIVATE
		"$<$<CONFIG:Debug>:"
			"_DEBUG"
		">"
		"$<$<CONFIG:Release>:"
			"NDEBUG"
		">"
        "$<$<BOOL:${BUILD_CROWD_CONTROL}>:ENABLE_CROWD_CONTROL>"
		"SPDLOG_ACTIVE_LEVEL=0;"
		"_CONSOLE;"
		"_CRT_SECURE_NO_WARNINGS;"
		"UNICODE;"
		"_UNICODE"
	)
endif()

if(USE_NETWORKING)
    if(MSVC)
        vcpkg_install_packages(sdl2-net)
    endif()
    find_package(SDL2_net REQUIRED)
    include_directories(${SDL2_NET_INCLUDE_DIRS})
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_NETWORKING)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    find_package(Ogg CONFIG REQUIRED)
    link_libraries(Ogg::ogg)

    find_package(Vorbis CONFIG REQUIRED)
    link_libraries(Vorbis::vorbisfile)
	set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<BOOL:${USE_NETWORKING}>:SDL2_net::SDL2_net-static>"
        "Ogg::ogg"
        "Vorbis::vorbis"
        "Vorbis::vorbisenc"
        "Vorbis::vorbisfile"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "NintendoSwitch")
    find_package(SDL2)
	find_package(Vorbis REQUIRED)
	link_libraries(Vorbis::vorbisfile)
	find_package(Ogg REQUIRED)
	link_libraries(Ogg::ogg)
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        -lglad
        SDL2::SDL2
        "Ogg::ogg"
        "Vorbis::vorbisfile"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "CafeOS")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:Debug>:-Wl,--wrap=abort>"
    )
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${DEVKITPRO}/portlibs/wiiu/include/
    )
else()
    find_package(Ogg REQUIRED)
    find_package(Vorbis REQUIRED)
 	set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<BOOL:${USE_NETWORKING}>:SDL2_net::SDL2_net>"
        "Ogg::ogg"
        "Vorbis::vorbis"
        "Vorbis::vorbisenc"
        "Vorbis::vorbisfile"
	)
    if(UNIX AND NOT APPLE)
        list(APPEND ADDITIONAL_LIBRARY_DEPENDENCIES dl)
    endif()
endif()

option(USE_STANDALONE "Build as a standalone executable" OFF)
option(BUILD_STORMLIB "Build with StormLib support" OFF)

option(BUILD_BK64 "Build with Banjo-Kazooie support" ON)
option(BUILD_PM64 "Build with Paper Mario support" OFF)
option(BUILD_SM64 "Build with Super Mario 64 support" OFF)
option(BUILD_MK64 "Build with Mario Kart 64 support" OFF)
option(BUILD_SF64 "Build with Star Fox 64 support" OFF)
option(BUILD_FZERO "Build with F-Zero X support" OFF)
option(BUILD_OOT "Build with Ocarina of Time support" OFF)
option(BUILD_MARIO_ARTIST "Build with Mario Artist support" OFF)
option(BUILD_NAUDIO "Build with NAudio support" OFF)
add_subdirectory(Torch)
target_link_libraries(${PROJECT_NAME} PRIVATE torch "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

if(CMAKE_SYSTEM_NAME MATCHES "NintendoSwitch")

nx_generate_nacp(${PROJECT_NAME}.nacp
   NAME "${PROJECT_NAME}"
   AUTHOR "${PROJECT_TEAM}"
   VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
)

nx_create_nro(${PROJECT_NAME}
    NACP ${PROJECT_NAME}.nacp
    ICON ${CMAKE_CURRENT_SOURCE_DIR}/nx-logo.jpg
)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.nro DESTINATION . COMPONENT ${PROJECT_NAME})
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
  if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
      target_compile_options(${PROJECT_NAME} PRIVATE
          $<$<CONFIG:Debug>:
              /W3;
              /Od;
              /MTd
          >
          $<$<CONFIG:Release>:
              /Oi;
              /Gy;
              /W3;
              /MT
          >
          /permissive-;
          /MP;
          # 32→64-bit pointer truncation warnings as errors
          /we4311;  # pointer truncation (e.g. void* to int)
          /we4312;  # conversion to pointer of greater size (e.g. int to void*)
          /we4133;  # incompatible pointer types
          /wd4200;  # we don't care about zero-sized flexible arrays
          ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
          ${DEFAULT_CXX_EXCEPTION_HANDLING}
      )
      if(ENABLE_ASAN)
          target_compile_options(${PROJECT_NAME} PRIVATE  $<$<CONFIG:Debug>:/Zi;>)
      else()
          target_compile_options(${PROJECT_NAME} PRIVATE  $<$<CONFIG:Debug>:/ZI;>)
      endif()
      target_compile_options(${PROJECT_NAME} PRIVATE  $<$<CONFIG:Release>:/Zi;>)
  elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
      target_compile_options(${PROJECT_NAME} PRIVATE
          $<$<CONFIG:Debug>:
              /MTd
          >
          $<$<CONFIG:Release>:
              /O2;
              /Oi;
              /Gy;
              /MT
          >
          /permissive-;
          /MP;
          /w;
          ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
          ${DEFAULT_CXX_EXCEPTION_HANDLING}
      )
  endif()
  if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
      target_link_options(${PROJECT_NAME} PRIVATE
          $<$<CONFIG:Debug>:
              /INCREMENTAL
          >
          $<$<CONFIG:Release>:
              /OPT:REF;
              /OPT:ICF;
              /INCREMENTAL:NO;
              /FORCE:MULTIPLE
          >
          /MANIFEST:NO;
          /DEBUG;
          /SUBSYSTEM:WINDOWS
      )
  elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
      target_link_options(${PROJECT_NAME} PRIVATE
          $<$<CONFIG:Debug>:
              /STACK:8777216
          >
          $<$<CONFIG:Release>:
              /OPT:REF;
              /OPT:ICF;
              /INCREMENTAL:NO;
              /FORCE:MULTIPLE
          >
          /MANIFEST:NO;
          /DEBUG;
          /SUBSYSTEM:WINDOWS
      )
  endif()

  # Remove /RTC and /Zi from msvc default flags to avoid conflicts with our /ZI and /Zi settings
  foreach (fentry
    CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELWITHDEBINFO
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELWITHDEBINFO
  )
    string (REGEX REPLACE "/RTC(su|[1su])" "" ${fentry} "${${fentry}}")
    string (REGEX REPLACE "/Z[iI]" "" ${fentry} "${${fentry}}")
  endforeach(fentry)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    if (CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
        target_compile_options(${PROJECT_NAME} PRIVATE
            -Wall -Wextra -Wno-error
            -fno-strict-aliasing
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
                -Wno-implicit-function-declaration
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
          -fno-strict-aliasing
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
  else()
      if(NOT DEFINED CPU_OPTION)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
          set(CPU_OPTION -msse2 -mfpmath=sse)
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
          set(CPU_OPTION -mcpu=native)
        endif()
      endif()

      target_compile_options(${PROJECT_NAME} PRIVATE
          -Wall -Wextra -Wno-error
          -fno-strict-aliasing
          -Wno-unused-parameter
          -Wno-unused-function
          -Wno-unused-variable
          -Wno-missing-field-initializers
          -Wno-parentheses
          -Wno-narrowing
          -Wno-missing-braces
          # 32→64-bit pointer truncation warnings as errors
          -Werror=pointer-to-int-cast
          -Werror=int-to-pointer-cast
          -Werror=int-conversion
          $<$<COMPILE_LANGUAGE:C>:
              -Werror=implicit-function-declaration
              -Werror=incompatible-pointer-types
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

if(NOT CMAKE_SYSTEM_NAME STREQUAL "NintendoSwitch")
include(ExternalProject)
ExternalProject_Add(TorchExternal
	PREFIX TorchExternal
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/Torch
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/Torch -DENABLE_ASAN=${ENABLE_ASAN}
)
#set_target_properties(TorchExternal PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
ExternalProject_Get_Property(TorchExternal install_dir)
if (CMAKE_CONFIGURATION_TYPES)
  set(TORCH_EXECUTABLE ${install_dir}/src/TorchExternal-build/$<CONFIGURATION>/torch)
else()
  set(TORCH_EXECUTABLE ${install_dir}/src/TorchExternal-build/torch)
endif()
add_custom_target(
    ExtractAssets
    DEPENDS TorchExternal
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${TORCH_EXECUTABLE} o2r baserom.z64
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/bk.o2r" "${CMAKE_BINARY_DIR}/bk.o2r"
)

add_custom_target(
    GeneratePortO2R
    DEPENDS TorchExternal
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_BINARY_DIR}/port_staging"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/port" "${CMAKE_BINARY_DIR}/port_staging"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/libultraship/src/fast/shaders" "${CMAKE_BINARY_DIR}/port_staging/shaders"
    COMMAND ${TORCH_EXECUTABLE} pack "${CMAKE_BINARY_DIR}/port_staging" lighthouse.o2r o2r
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/lighthouse.o2r" "${CMAKE_BINARY_DIR}/lighthouse.o2r"
)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  install(FILES "${CMAKE_BINARY_DIR}/lighthouse.o2r" DESTINATION . COMPONENT ${PROJECT_NAME})
  install(DIRECTORY "${CMAKE_SOURCE_DIR}/assets/" DESTINATION ./assets COMPONENT ${PROJECT_NAME})
  install(FILES "${CMAKE_BINARY_DIR}/config.yml" DESTINATION . COMPONENT ${PROJECT_NAME})
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
add_custom_target(CreateOSXIcons
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/macosx/lighthouse.iconset
    COMMAND sips -z 16 16     logo.png --out ${CMAKE_BINARY_DIR}/macosx/lighthouse.iconset/icon_16x16.png
    COMMAND sips -z 32 32     logo.png --out ${CMAKE_BINARY_DIR}/macosx/lighthouse.iconset/icon_16x16@2x.png
    COMMAND sips -z 32 32     logo.png --out ${CMAKE_BINARY_DIR}/macosx/lighthouse.iconset/icon_32x32.png
    COMMAND sips -z 64 64     logo.png --out ${CMAKE_BINARY_DIR}/macosx/lighthouse.iconset/icon_32x32@2x.png
    COMMAND sips -z 128 128   logo.png --out ${CMAKE_BINARY_DIR}/macosx/lighthouse.iconset/icon_128x128.png
    COMMAND sips -z 256 256   logo.png --out ${CMAKE_BINARY_DIR}/macosx/lighthouse.iconset/icon_128x128@2x.png
    COMMAND sips -z 256 256   logo.png --out ${CMAKE_BINARY_DIR}/macosx/lighthouse.iconset/icon_256x256.png
    COMMAND sips -z 512 512   logo.png --out ${CMAKE_BINARY_DIR}/macosx/lighthouse.iconset/icon_256x256@2x.png
    COMMAND sips -z 512 512   logo.png --out ${CMAKE_BINARY_DIR}/macosx/lighthouse.iconset/icon_512x512.png
    COMMAND cp                logo.png ${CMAKE_BINARY_DIR}/macosx/lighthouse.iconset/icon_512x512@2x.png
    COMMAND iconutil -c icns -o ${CMAKE_BINARY_DIR}/macosx/lighthouse.icns ${CMAKE_BINARY_DIR}/macosx/lighthouse.iconset
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Creating OSX icons ..."
    )
    add_dependencies(${PROJECT_NAME} CreateOSXIcons)
    configure_file("${CMAKE_SOURCE_DIR}/Info.plist" "${CMAKE_BINARY_DIR}/Info.plist" COPYONLY)

    file(DOWNLOAD
        https://raw.githubusercontent.com/mdqinc/SDL_GameControllerDB/master/gamecontrollerdb.txt
        ${CMAKE_BINARY_DIR}/gamecontrollerdb.txt)

    INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ../MacOS COMPONENT ${PROJECT_NAME})
    INSTALL(FILES ${CMAKE_BINARY_DIR}/gamecontrollerdb.txt DESTINATION ../MacOS COMPONENT ${PROJECT_NAME})
    INSTALL(FILES ${CMAKE_BINARY_DIR}/lighthouse.o2r DESTINATION ../Resources COMPONENT ${PROJECT_NAME})
    INSTALL(FILES ${CMAKE_BINARY_DIR}/config.yml DESTINATION ../Resources COMPONENT ${PROJECT_NAME})
    INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/assets/ DESTINATION ../Resources/assets COMPONENT ${PROJECT_NAME})

    install(CODE "
        include(BundleUtilities)
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/../MacOS/Lighthouse\" \"\" \"${dirs}\")
    ")
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY APPIMAGE_DESKTOP_FILE_TERMINAL YES)
set_property(TARGET ${PROJECT_NAME} PROPERTY APPIMAGE_DESKTOP_FILE "${CMAKE_SOURCE_DIR}/Lighthouse.desktop")
set_property(TARGET ${PROJECT_NAME} PROPERTY APPIMAGE_ICON_FILE "${CMAKE_SOURCE_DIR}/logo.png")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(CPACK_GENERATOR "External")
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows|NintendoSwitch|CafeOS")
    set(CPACK_GENERATOR "ZIP")
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(CPACK_GENERATOR "Bundle")
endif()

set(CPACK_PROJECT_CONFIG_FILE ${CMAKE_SOURCE_DIR}/cmake/configure-packaging.cmake)
include(cmake/packaging.cmake)
