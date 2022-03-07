cmake_minimum_required(VERSION 3.21)

set(CMAKE_SYSTEM_NAME elf)
#set(CMAKE_SYSTEM_PROCESSOR i686)
#set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

#execute_process(
#    COMMAND
#        ${CMAKE_ASM_COMPILER} -Wa,--divide -g -c ${CMAKE_SOURCE_DIR}/src/i686/crti.s -o ${CMAKE_BINARY_DIR}/crti.o
#)
#execute_process(
#    COMMAND
#        ${CMAKE_ASM_COMPILER} -Wa,--divide -g -c ${CMAKE_SOURCE_DIR}/src/i686/crtn.s -o ${CMAKE_BINARY_DIR}/crtn.o
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
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -nostdlib -ffreestanding -Wall -Wextra -pedantic -pedantic-errors")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostdlib -ffreestanding -Wall -Wextra -pedantic -pedantic-errors -fno-exceptions") # -fno-rtti

set(CMAKE_ASM_LINK_EXECUTABLE "<CMAKE_ASM_COMPILER> <FLAGS> <CMAKE_ASM_LINK_FLAGS> <LINK_FLAGS> ${CMAKE_BINARY_DIR}/crti.o ${BOOT_CRTBEGIN} <OBJECTS> -o <TARGET> <LINK_LIBRARIES> ${BOOT_CRTEND} ${CMAKE_BINARY_DIR}/crtn.o")
set(CMAKE_C_LINK_EXECUTABLE   "<CMAKE_C_COMPILER>   <FLAGS> <CMAKE_C_LINK_FLAGS>   <LINK_FLAGS> ${CMAKE_BINARY_DIR}/crti.o ${BOOT_CRTBEGIN} <OBJECTS> -o <TARGET> <LINK_LIBRARIES> ${BOOT_CRTEND} ${CMAKE_BINARY_DIR}/crtn.o")
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> ${CMAKE_BINARY_DIR}/crti.o ${BOOT_CRTBEGIN} <OBJECTS> -o <TARGET> <LINK_LIBRARIES> ${BOOT_CRTEND} ${CMAKE_BINARY_DIR}/crtn.o")

project(boot LANGUAGES ASM C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BOOT_LINKER_SCRIPT src/ldscripts/i686.ld)

add_executable(boot
    src/boot/Conout.cpp
    src/boot/conout.h
    src/boot/Conout.hpp
    src/boot/endian.c
    src/boot/endian.h
    src/boot/gpt.h
    src/boot/IBlockDevice.h
    src/boot/qsort.c
    src/boot/UniquePtr.hpp
    src/boot/util.c
    src/boot/util.h
    src/btstdbeg.h
    src/btstdend.h
    src/ext2/ext2.h
    src/ext2/File.h
    src/ext2/File.cpp
    src/ext2/Driver.h
    src/ext2/Driver.cpp
    src/i686/Conout_impl.h
    src/i686/Conout_impl.cpp
    src/i686/alloc.c
    src/i686/alloc.h
    src/i686/bios_disk.h
    src/i686/bios_disk.s
    src/i686/bios_kbrd.h
    src/i686/bios_kbrd.s
    src/i686/bios_video.h
    src/i686/bios_video.s
    src/i686/exit.c
    src/i686/init.c
    src/i686/init.h
    src/i686/init.s
    src/i686/membios.h
    src/i686/membios.s
    src/i686/memop.c
    src/i686/memop.s
    src/i686/mode_switch.c
    src/i686/mode_switch.h
    src/i686/mode_switch.s
    src/i686/new.cpp
    src/i686/PartitionDevice.h
    src/i686/PartitionDevice.cpp
    src/i686/processor.c
    src/i686/processor.h
    src/i686/vgacon.h
    src/i686/vgacon.s
    src/main.cpp
    src/string.h
    src/stdlib.h
    ${BOOT_LINKER_SCRIPT}
)
target_compile_options(boot PRIVATE
    -mgeneral-regs-only
)
target_include_directories(boot PRIVATE
    src
    ${CMAKE_BINARY_DIR}/include
)
target_link_options(boot PRIVATE
    -T ${CMAKE_SOURCE_DIR}/${BOOT_LINKER_SCRIPT}
)
target_link_libraries(boot PRIVATE supc++ gcc)
add_dependencies(boot crti crtn)
set_target_properties(boot PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/${BOOT_LINKER_SCRIPT})

add_custom_target(crti
    ALL ${CMAKE_ASM_COMPILER} -Wa,--divide -g -c ${CMAKE_SOURCE_DIR}/src/i686/crti.s -o crti.o
    SOURCES
        src/i686/crti.s
    BYPRODUCTS
        crti.o
)
add_custom_target(crtn
    ALL ${CMAKE_ASM_COMPILER} -Wa,--divide -g -c ${CMAKE_SOURCE_DIR}/src/i686/crtn.s -o crtn.o
    SOURCES
        src/i686/crtn.s
    BYPRODUCTS
        crtn.o
)

add_custom_target("boot.bin" ALL ${CMAKE_OBJCOPY} -O binary boot boot.bin BYPRODUCTS boot.bin)
add_dependencies("boot.bin" boot)
