cmake_minimum_required(VERSION 3.21)

set(CMAKE_SYSTEM_NAME elf)
#set(CMAKE_SYSTEM_PROCESSOR i686)
#set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

#execute_process(
#    COMMAND
#        ${CMAKE_ASM_COMPILER} -Wa,--divide -c ${CMAKE_SOURCE_DIR}/src/i686/crti.s -o ${CMAKE_BINARY_DIR}/crti.o
#)
#execute_process(
#    COMMAND
#        ${CMAKE_ASM_COMPILER} -Wa,--divide -c ${CMAKE_SOURCE_DIR}/src/i686/crtn.s -o ${CMAKE_BINARY_DIR}/crtn.o
#)

execute_process(
    COMMAND
        ${CMAKE_CXX_COMPILER} -print-file-name=crtbegin.o
    OUTPUT_VARIABLE BOOT_CRTBEGIN
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND
        ${CMAKE_CXX_COMPILER} -print-file-name=crtend.o
    OUTPUT_VARIABLE BOOT_CRTEND
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -nostdlib -Wa,--divide")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -nostdlib -ffreestanding")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostdlib -ffreestanding") #-fno-exceptions -fno-rtti

set(CMAKE_ASM_LINK_EXECUTABLE "<CMAKE_ASM_COMPILER> <FLAGS> <CMAKE_ASM_LINK_FLAGS> <LINK_FLAGS> ${CMAKE_BINARY_DIR}/crti.o ${BOOT_CRTBEGIN} <OBJECTS> -o <TARGET> <LINK_LIBRARIES> ${BOOT_CRTEND} ${CMAKE_BINARY_DIR}/crtn.o")
set(CMAKE_C_LINK_EXECUTABLE   "<CMAKE_C_COMPILER>   <FLAGS> <CMAKE_C_LINK_FLAGS>   <LINK_FLAGS> ${CMAKE_BINARY_DIR}/crti.o ${BOOT_CRTBEGIN} <OBJECTS> -o <TARGET> <LINK_LIBRARIES> ${BOOT_CRTEND} ${CMAKE_BINARY_DIR}/crtn.o")
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> ${CMAKE_BINARY_DIR}/crti.o ${BOOT_CRTBEGIN} <OBJECTS> -o <TARGET> <LINK_LIBRARIES> ${BOOT_CRTEND} ${CMAKE_BINARY_DIR}/crtn.o")

project(boot LANGUAGES ASM C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BOOT_LINKER_SCRIPT src/ldscripts/i686.ld)

add_executable(boot
    src/i686/alloc.c
#    src/i686/ctordtor.c
    src/i686/init.s
    src/i686/membios.h
    src/i686/membios.s
    src/i686/memop.c
    src/i686/memop.s
    src/i686/mode_switch.c
    src/i686/mode_switch.h
    src/i686/mode_switch.s
    src/i686/processor.h
    src/i686/video_bios.h
    src/i686/video_bios.s
    src/main.cpp
    src/testxcpt.cpp
    ${BOOT_LINKER_SCRIPT}
)
target_compile_options(boot PRIVATE
    -Wall
    -Wextra
    -pedantic
    -pedantic-errors
    -mgeneral-regs-only
)
target_link_options(boot PRIVATE
    -T ${CMAKE_SOURCE_DIR}/${BOOT_LINKER_SCRIPT}
)
target_link_libraries(boot PRIVATE supc++ gcc)
add_dependencies(boot crti crtn)
set_target_properties(boot PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/${BOOT_LINKER_SCRIPT})

add_custom_target(crti
    ALL ${CMAKE_ASM_COMPILER} -Wa,--divide -c ${CMAKE_SOURCE_DIR}/src/i686/crti.s -o crti.o
    SOURCES
        src/i686/crti.s
    BYPRODUCTS
        crti.o
)
add_custom_target(crtn
    ALL ${CMAKE_ASM_COMPILER} -Wa,--divide -c ${CMAKE_SOURCE_DIR}/src/i686/crtn.s -o crtn.o
    SOURCES
        src/i686/crtn.s
    BYPRODUCTS
        crtn.o
)

add_custom_target("boot.bin" ALL ${CMAKE_OBJCOPY} -O binary boot boot.bin)
add_dependencies("boot.bin" boot)
