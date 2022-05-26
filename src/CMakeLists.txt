cmake_minimum_required(VERSION 3.21)

set(CMAKE_SYSTEM_NAME elf)
#set(CMAKE_SYSTEM_PROCESSOR i686)
#set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

#execute_process(
#    COMMAND
#        ${CMAKE_ASM_COMPILER} -Wa,--divide -g -c ${CMAKE_CURRENT_SOURCE_DIR}/src/i686/crti.s -o ${CMAKE_CURRENT_BINARY_DIR}/crti.o
#)
#execute_process(
#    COMMAND
#        ${CMAKE_ASM_COMPILER} -Wa,--divide -g -c ${CMAKE_CURRENT_SOURCE_DIR}/src/i686/crtn.s -o ${CMAKE_CURRENT_BINARY_DIR}/crtn.o
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

set(CMAKE_ASM_LINK_EXECUTABLE "<CMAKE_ASM_COMPILER> <FLAGS> <CMAKE_ASM_LINK_FLAGS> <LINK_FLAGS> ${CMAKE_CURRENT_BINARY_DIR}/crti.o ${BOOT_CRTBEGIN} <OBJECTS> -o <TARGET> <LINK_LIBRARIES> ${BOOT_CRTEND} ${CMAKE_CURRENT_BINARY_DIR}/crtn.o")
set(CMAKE_C_LINK_EXECUTABLE   "<CMAKE_C_COMPILER>   <FLAGS> <CMAKE_C_LINK_FLAGS>   <LINK_FLAGS> ${CMAKE_CURRENT_BINARY_DIR}/crti.o ${BOOT_CRTBEGIN} <OBJECTS> -o <TARGET> <LINK_LIBRARIES> ${BOOT_CRTEND} ${CMAKE_CURRENT_BINARY_DIR}/crtn.o")
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> ${CMAKE_CURRENT_BINARY_DIR}/crti.o ${BOOT_CRTBEGIN} <OBJECTS> -o <TARGET> <LINK_LIBRARIES> ${BOOT_CRTEND} ${CMAKE_CURRENT_BINARY_DIR}/crtn.o")

project(boot LANGUAGES ASM C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BOOT_LINKER_SCRIPT ldscripts/i686.ld)

add_library(boot_std STATIC
    boot_std/src/qsort.c
    boot_std/include/btstdbeg.h
    boot_std/include/btstdend.h
    boot_std/include/string.h
    boot_std/include/stdlib.h
)
target_include_directories(boot_std PUBLIC boot_std/include)
target_link_libraries(boot_std PUBLIC boot_std_int supc++ gcc)

add_library(generic STATIC
    generic/include/boot/conout.h
    generic/include/boot/Conout.hpp
    generic/include/boot/Conout_preimpl.hpp
    generic/include/boot/endian.h
    generic/include/boot/gpt.h
    generic/include/boot/ioInterface.h
    generic/include/boot/StatusCode.hpp
    generic/include/boot/UniquePtr.hpp
    generic/include/boot/util.h
    generic/include/boot/util.hpp
    generic/src/endian.c
    generic/src/util.c
    ext2/ext2.h
    ext2/Directory.h
    ext2/Directory.cpp
    ext2/Driver.h
    ext2/Driver.cpp
    ext2/File.h
    ext2/File.cpp
)
target_include_directories(generic PUBLIC generic/include)
target_link_libraries(generic PUBLIC boot_std)

add_library(platform_i686 STATIC
    platform/i686/Conout_impl.h
    platform/i686/Conout_impl.cpp
    platform/i686/alloc.c
    platform/i686/alloc.h
    platform/i686/bios_disk.h
    platform/i686/bios_disk.s
    platform/i686/bios_kbrd.h
    platform/i686/bios_kbrd.s
    platform/i686/bios_video.h
    platform/i686/bios_video.s
    platform/i686/exit.c
    platform/i686/init.c
    platform/i686/init.h
    platform/i686/init.s
    platform/i686/membios.h
    platform/i686/membios.s
    platform/i686/memop.c
    platform/i686/memop.s
    platform/i686/mode_switch.c
    platform/i686/mode_switch.h
    platform/i686/mode_switch.s
    platform/i686/new.cpp
    platform/i686/PartitionDevice.h
    platform/i686/PartitionDevice.cpp
    platform/i686/processor.c
    platform/i686/processor.h
    platform/i686/vgacon.h
    platform/i686/vgacon.s
)
target_link_libraries(platform_i686 PUBLIC generic)

add_library(platform ALIAS platform_i686)

add_executable(boot
    boot/main.cpp
    ${BOOT_LINKER_SCRIPT}
)
#target_compile_options(boot PRIVATE
#    -mgeneral-regs-only
#)
target_include_directories(boot PRIVATE
    .
    ${CMAKE_CURRENT_BINARY_DIR}/include
)
target_link_options(boot PRIVATE
    -T ${CMAKE_CURRENT_SOURCE_DIR}/${BOOT_LINKER_SCRIPT}
)
target_link_libraries(boot PRIVATE platform)
add_dependencies(boot crti crtn)
set_target_properties(boot PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${BOOT_LINKER_SCRIPT})

add_custom_target(crti
    ALL ${CMAKE_ASM_COMPILER} -Wa,--divide -g -c ${CMAKE_CURRENT_SOURCE_DIR}/platform/i686/crti.s -o crti.o
    SOURCES
        platform/i686/crti.s
    BYPRODUCTS
        crti.o
)
add_custom_target(crtn
    ALL ${CMAKE_ASM_COMPILER} -Wa,--divide -g -c ${CMAKE_CURRENT_SOURCE_DIR}/platform/i686/crtn.s -o crtn.o
    SOURCES
        platform/i686/crtn.s
    BYPRODUCTS
        crtn.o
)

add_custom_target("boot.bin" ALL ${CMAKE_OBJCOPY} -O binary boot boot.bin BYPRODUCTS boot.bin)
add_dependencies("boot.bin" boot)
