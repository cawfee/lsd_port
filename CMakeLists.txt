cmake_minimum_required(VERSION 3.16)
project(lsd_port LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if(NOT DEFINED LSD_TARGET)
  set(LSD_TARGET host)
endif()
message(STATUS "lsd_port target: ${LSD_TARGET}")

# Project sources
file(GLOB_RECURSE LSD_GAME_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/lsd/*.c")
file(GLOB LSD_PSX_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/psx/*.c")

# Platform dependant sources
if(LSD_TARGET STREQUAL "ps1")
  set(LSD_PLATFORM_DIR "src/platform/ps1")
else()
  # Default to SDL2
  set(LSD_PLATFORM_DIR "src/platform/sdl")
endif()

file(GLOB LSD_PLATFORM_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/${LSD_PLATFORM_DIR}/*.c")

if(MSVC)
  add_compile_options(/W3)
else()
  add_compile_options(-Wall -Wextra)
endif()

if(LSD_PSX_SOURCES)
  add_library(lsd_psx STATIC ${LSD_PSX_SOURCES})
  target_include_directories(lsd_psx PUBLIC "${PROJECT_SOURCE_DIR}/src")
endif()

if(LSD_GAME_SOURCES)
  add_library(lsd_game STATIC ${LSD_GAME_SOURCES})
  target_include_directories(lsd_game PUBLIC "${PROJECT_SOURCE_DIR}/src")

  if(TARGET lsd_psx)
    target_link_libraries(lsd_game PUBLIC lsd_psx)
  endif()
endif()

if(LSD_PLATFORM_SOURCES)
  add_executable(lsd ${LSD_PLATFORM_SOURCES})
  foreach(lib lsd_game lsd_psx)
    if(TARGET ${lib})
      target_link_libraries(lsd PRIVATE ${lib})
    endif()
  endforeach()
else()
  message(STATUS "${LSD_PLATFORM_DIR} holds no .c files yet -- nothing to build (this is fine)")
endif()

if(LSD_TARGET STREQUAL "host" OR LSD_TARGET STREQUAL "vita")
  find_package(SDL2 QUIET)
  if(NOT TARGET SDL2::SDL2 AND NOT CMAKE_CROSSCOMPILING)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
      pkg_check_modules(SDL2 QUIET IMPORTED_TARGET sdl2)
    endif()
  endif()

  # Find SDL2
  if(TARGET SDL2::SDL2)
    set(LSD_SDL SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    set(LSD_SDL PkgConfig::SDL2)
  else()
    message(FATAL_ERROR "SDL2 was not found.")
  endif()

  if(TARGET lsd)
    if(TARGET SDL2::SDL2main)
      target_link_libraries(lsd PRIVATE SDL2::SDL2main)
    endif()
    target_link_libraries(lsd PRIVATE ${LSD_SDL})
    target_compile_definitions(lsd PRIVATE LSD_PLATFORM_SDL)

    if(WIN32 AND SDL2_SDL2_FOUND)
      add_custom_command(TARGET lsd POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:SDL2::SDL2>" "$<TARGET_FILE_DIR:lsd>"
        COMMENT "Copying SDL2.dll next to lsd${CMAKE_EXECUTABLE_SUFFIX}")
    endif()
  endif()
endif()

# if(LSD_TARGET STREQUAL "vita")
#   if(TARGET lsd AND DEFINED ENV{VITASDK} AND EXISTS "$ENV{VITASDK}/share/vita.cmake")
#     include("$ENV{VITASDK}/share/vita.cmake")
#     vita_create_self(lsd.self lsd UNSAFE)
#   endif()
# endif()

# if(LSD_TARGET STREQUAL "ps1")
#   if(TARGET lsd)
#     target_compile_definitions(lsd PRIVATE LSD_PLATFORM_PS1)
#     set_target_properties(lsd PROPERTIES SUFFIX .elf)

#     find_program(LSD_ELF2PSX NAMES elf2psx psxexe
#       HINTS "${PSN00BSDK_ROOT}/bin" "$ENV{PSN00BSDK}/bin")
#     if(LSD_ELF2PSX)
#       add_custom_command(TARGET lsd POST_BUILD
#         COMMAND ${LSD_ELF2PSX} $<TARGET_FILE:lsd> ${CMAKE_BINARY_DIR}/SLPS_015.56)
#     else()
#       message(STATUS "ps1: no elf2psx tool found yet -- .elf only (packaging TODO)")
#     endif()
#   endif()
# endif()
