cmake_minimum_required(VERSION 3.1)
project(soulless)

# Set c++11
# https://stackoverflow.com/questions/10851247/how-to-activate-c-11-in-cmake
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif ()
set (CMAKE_CXX_STANDARD 14)

# nice hierarchical structure in MSVC
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Find OS
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(IS_OS_MAC 1)
  
  # Check for Apple Silicon
  execute_process(
    COMMAND uname -m
    OUTPUT_VARIABLE ARCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  
  if(ARCH MATCHES "arm64")
    message(STATUS "Configuring for Apple Silicon with Homebrew LLVM")
    
    # Set LLVM paths explicitly
    set(LLVM_PATH "/opt/homebrew/opt/llvm")
    
    # Set compilers explicitly
    set(CMAKE_C_COMPILER "${LLVM_PATH}/bin/clang")
    set(CMAKE_CXX_COMPILER "${LLVM_PATH}/bin/clang++")
    
    # Add include paths
    include_directories(SYSTEM "${LLVM_PATH}/include")
    include_directories(SYSTEM "${LLVM_PATH}/include/c++/v1")
    
    # Add library path
    link_directories("${LLVM_PATH}/lib")
    link_directories("${LLVM_PATH}/lib/c++")
    
    # Set compiler and linker flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${LLVM_PATH}/lib -Wl,-rpath,${LLVM_PATH}/lib")
  endif()
  
  # Regular Mac paths (keep these for compatibility)
  include_directories(/usr/local/include)
  link_directories(/usr/local/lib)
  
  # Standard Homebrew paths
  include_directories(/opt/homebrew/include)
  link_directories(/opt/homebrew/lib)
  
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(IS_OS_LINUX 1)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(IS_OS_WINDOWS 1)
else()
  message(FATAL_ERROR "OS ${CMAKE_SYSTEM_NAME} was not recognized")
endif()

# Create executable target

# Generate the shader folder location to the header
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ext/project_path.hpp.in" "${CMAKE_CURRENT_SOURCE_DIR}/ext/project_path.hpp")

# You can switch to use the file GLOB for simplicity but at your own risk
file(GLOB_RECURSE SOURCE_FILES src/*.cpp include/*.hpp)

# external libraries will be installed into /usr/local/include and /usr/local/lib but that folder is not automatically included in the search on MACs
if (IS_OS_MAC)
  include_directories(/usr/local/include)
  link_directories(/usr/local/lib)
  # 2024-09-24 - added for M-series Mac's
  include_directories(/opt/homebrew/include)
  link_directories(/opt/homebrew/lib)
endif()

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC include/ src/)

# Added this so policy CMP0065 doesn't scream
set_target_properties(${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS 0)

# External header-only libraries in the ext/
target_include_directories(${PROJECT_NAME} PUBLIC ext/stb_image/)
target_include_directories(${PROJECT_NAME} PUBLIC ext/gl3w)

# Find OpenGL
find_package(OpenGL REQUIRED)

if (OPENGL_FOUND)
   target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})
   target_link_libraries(${PROJECT_NAME} PUBLIC ${OPENGL_gl_LIBRARY})
endif()

set(glm_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glm/cmake/glm) # if necessary
find_package(glm REQUIRED)

# FFmpeg configuration
if (IS_OS_WINDOWS)
    # Windows-specific FFmpeg setup
    set(FFMPEG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/ext/ffmpeg")
    set(FFMPEG_INCLUDE_DIRS "${FFMPEG_ROOT}/include")
    set(FFMPEG_LIBRARY_DIRS "${FFMPEG_ROOT}/lib")

    if (${CMAKE_SIZEOF_VOID_P} MATCHES "8")
        # 64-bit Windows
        set(FFMPEG_LIBRARIES
            "${FFMPEG_LIBRARY_DIRS}/avcodec.lib"
            "${FFMPEG_LIBRARY_DIRS}/avformat.lib"
            "${FFMPEG_LIBRARY_DIRS}/avutil.lib"
            "${FFMPEG_LIBRARY_DIRS}/swscale.lib"
            "${FFMPEG_LIBRARY_DIRS}/swresample.lib"
            "${FFMPEG_LIBRARY_DIRS}/avdevice.lib"
        )
        
        # Copy DLLs to output directory
        set(FFMPEG_DLLS
            "${FFMPEG_ROOT}/bin/avcodec-61.dll"
            "${FFMPEG_ROOT}/bin/avformat-61.dll"
            "${FFMPEG_ROOT}/bin/avutil-59.dll"
            "${FFMPEG_ROOT}/bin/swscale-8.dll"
            "${FFMPEG_ROOT}/bin/swresample-5.dll"
            "${FFMPEG_ROOT}/bin/avdevice-61.dll"
        )

        foreach(DLL ${FFMPEG_DLLS})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${DLL}"
                    "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
        endforeach()
    else()
        # 32-bit Windows configuration if needed
    endif()
    
    target_include_directories(${PROJECT_NAME} PUBLIC ${FFMPEG_INCLUDE_DIRS})
else()
    # Unix-like systems (Linux, macOS) use pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
        libavcodec
        libavformat
        libavutil
        libswscale
        libswresample
        libavdevice
    )
endif()

# GLFW, SDL2 could be precompiled (on windows) or installed by a package manager (on OSX and Linux)
if (IS_OS_LINUX OR IS_OS_MAC)
    # Try to find packages rather than to use the precompiled ones
    # Since we're on OSX or Linux, we can just use pkgconfig.
    find_package(PkgConfig REQUIRED)

    pkg_search_module(GLFW REQUIRED glfw3)

    pkg_search_module(SDL2 REQUIRED sdl2)
    pkg_search_module(SDL2MIXER REQUIRED SDL2_mixer)

    # Link Frameworks on OSX
    if (IS_OS_MAC)
       find_library(COCOA_LIBRARY Cocoa)
       find_library(CF_LIBRARY CoreFoundation)
       target_link_libraries(${PROJECT_NAME} PUBLIC ${COCOA_LIBRARY} ${CF_LIBRARY})
    endif()

    # Increase warning level
    target_compile_options(${PROJECT_NAME} PUBLIC "-Wall")
elseif (IS_OS_WINDOWS)
# https://stackoverflow.com/questions/17126860/cmake-link-precompiled-library-depending-on-os-and-architecture
    set(GLFW_FOUND TRUE)
    set(SDL2_FOUND TRUE)

    set(GLFW_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/include")
    set(SDL2_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/include/SDL")

    if (${CMAKE_SIZEOF_VOID_P} MATCHES "8")
        set(GLFW_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/lib/glfw3dll-x64.lib")
        set(SDL2_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2-x64.lib")
        set(SDL2MIXER_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2_mixer-x64.lib")

        set(GLFW_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/lib/glfw3-x64.dll")
        set(SDL2_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2-x64.dll")
        set(SDL2MIXER_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2_mixer-x64.dll")
    else()
        set(GLFW_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/lib/glfw3dll-x86.lib")
        set(SDL2_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2-x86.lib")
        set(SDL2MIXER_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2_mixer-x86.lib")

        set(GLFW_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/lib/glfw3-x86.dll")
        set(SDL2_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2-x86.dll")
        set(SDL2MIXER_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2_mixer-x86.dll")
    endif()

    # FreeType - Windows-specific (x64 only)
    set (FREETYPE_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/ext/freetype/include")
    set (FREETYPE_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/ext/freetype/release static/vs2015-2022/win64/freetype.lib")

    # Copy and rename dlls
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GLFW_DLL}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/glfw3.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_DLL}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL2.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2MIXER_DLL}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL2_mixer.dll")

    target_compile_options(${PROJECT_NAME} PUBLIC
        # increase warning level
        "/W4"

        # Turn warning "not all control paths return a value" into an error
        "/we4715"

        # use sane exception handling, rather than trying to catch segfaults and allowing resource
        # leaks and UB. Yup... See "Default exception handling behavior" at
        # https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model?view=vs-2019
        "/EHsc"

        # turn warning C4239 (non-standard extension that allows temporaries to be bound to
        # non-const references, yay microsoft) into an error
        "/we4239"
    )
endif()

# Can't find the include and lib. Quit.
if (NOT GLFW_FOUND OR NOT SDL2_FOUND)
   if (NOT GLFW_FOUND)
      message(FATAL_ERROR "Can't find GLFW." )
   else ()
      message(FATAL_ERROR "Can't find SDL2." )
   endif()
endif()

find_package(Freetype REQUIRED)

if(TARGET Freetype AND NOT TARGET Freetype::Freetype)
     add_library(Freetype::Freetype ALIAS freetype)
endif()
 
if(NOT TARGET Freetype::Freetype)
     message(FATAL_ERROR "Can't find FreeType (fonts)." )
endif()
 
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/ext/freetype/include")

target_include_directories(${PROJECT_NAME} PUBLIC ${GLFW_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PUBLIC
        ${GLFW_LIBRARIES}
        ${SDL2_LIBRARIES}
        ${SDL2MIXER_LIBRARIES}
        glm::glm
        ${FREETYPE_LIBRARY}
        $<$<NOT:$<BOOL:${IS_OS_WINDOWS}>>:PkgConfig::FFMPEG>
        $<$<BOOL:${IS_OS_WINDOWS}>:${FFMPEG_LIBRARIES}>
)

# Needed to add this
if(IS_OS_LINUX)
  target_link_libraries(${PROJECT_NAME} PUBLIC glfw ${CMAKE_DL_LIBS})
endif()