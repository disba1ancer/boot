cmake_minimum_required(VERSION 3.5)

set(CMAKE_SYSTEM_NAME elf)
#set(CMAKE_SYSTEM_PROCESSOR i686)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(boot LANGUAGES ASM C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BOOT_LINKER_SCRIPT src/ldscripts/i686.ld)

add_executable(boot
    src/i686/ctordtor.c
    src/i686/init.s
    src/i686/fini.s
    src/i686/mode_switch.c
    src/i686/mode_switch.h
    src/i686/mode_switch.s
    src/i686/processor.h
    src/i686/video_bios.h
    src/i686/video_bios.s
    src/main.cpp
    ${BOOT_LINKER_SCRIPT}
)
target_compile_options(boot PRIVATE
    -ffreestanding
    -Wall
    -Wextra
    -pedantic
    -pedantic-errors
    -mgeneral-regs-only
)
target_link_options(boot PRIVATE
    -nostdlib
    -lgcc
    -T ${CMAKE_SOURCE_DIR}/${BOOT_LINKER_SCRIPT}
)
set_target_properties(boot PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/${BOOT_LINKER_SCRIPT})

add_custom_target("boot.bin" ALL objcopy -O binary boot boot.bin)
add_dependencies("boot.bin" boot)
