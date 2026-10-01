# set(CMAKE_SYSTEM_NAME Generic)
# set(CMAKE_SYSTEM_PROCESSOR mipsel)
# set(LSD_TARGET ps1)

# find_program(CMAKE_C_COMPILER mipsel-none-elf-gcc)
# if(NOT CMAKE_C_COMPILER)
#   message(FATAL_ERROR
#     "mipsel-none-elf-gcc not found. Install PSn00bSDK (with its toolchain) "
#     "and make sure its bin/ is on PATH.")
# endif()

# # No hosted libc to link a test program against.
# set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# if(NOT DEFINED PSN00BSDK_ROOT)
#   if(DEFINED ENV{PSN00BSDK})
#     set(PSN00BSDK_ROOT "$ENV{PSN00BSDK}")
#   elseif(DEFINED ENV{PSN00BSDK_ROOT})
#     set(PSN00BSDK_ROOT "$ENV{PSN00BSDK_ROOT}")
#   endif()
# endif()

# set(LSD_PS1_CFLAGS "-march=r3000 -mtune=r3000 -msoft-float -mno-abicalls -G0 -ffreestanding")
# set(CMAKE_C_FLAGS_INIT "${LSD_PS1_CFLAGS}")
# set(CMAKE_EXE_LINKER_FLAGS_INIT "-nostdlib")

# if(PSN00BSDK_ROOT)
#   include_directories(SYSTEM "${PSN00BSDK_ROOT}/mipsel-none-elf/include")
#   link_directories("${PSN00BSDK_ROOT}/mipsel-none-elf/lib")
# endif()
