add_library(boot_std OBJECT
    boot_std/src/qsort.c
    boot_std/include/btstdbeg.h
    boot_std/include/btstdend.h
    boot_std/include/string.h
    boot_std/include/stdlib.h
)
target_include_directories(boot_std PUBLIC boot_std/include)

add_library(generic OBJECT
    elf/elf.h
    ext2/Directory.cpp
    ext2/Directory.h
    ext2/Driver.cpp
    ext2/Driver.h
    ext2/File.cpp
    ext2/File.h
    ext2/ext2.h
    generic/include/boot/Conout.hpp
    generic/include/boot/Conout_preimpl.hpp
    generic/include/boot/StatusCode.hpp
    generic/include/boot/UniquePtr.hpp
    generic/include/boot/conout.h
    generic/include/boot/data.h
    generic/include/boot/endian.h
    generic/include/boot/gpt.h
    generic/include/boot/ioInterface.h
    generic/include/boot/util.h
    generic/include/boot/util.hpp
    generic/include/boot/virtual_alloc.h
    generic/src/endian.c
    generic/src/util.c
)
target_include_directories(generic PUBLIC generic/include)
target_link_libraries(generic PUBLIC boot_std)

add_library(platform_i686 OBJECT
    platform/i686/Conout_impl.h
    platform/i686/Conout_impl.cpp
    platform/i686/alloc.c
    platform/i686/alloc.h
    platform/i686/virtual_alloc.c
    platform/i686/bios_disk.h
    platform/i686/bios_disk.s
    platform/i686/bios_kbrd.h
    platform/i686/bios_kbrd.s
    platform/i686/bios_video.h
    platform/i686/bios_video.s
    platform/i686/exit.c
    platform/i686/i686.ld
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
    platform/i686/specs.txt
    platform/i686/vgacon.h
    platform/i686/vgacon.s
)
target_link_libraries(platform_i686 PUBLIC generic supc++ gcc)
target_link_options(platform_i686 INTERFACE
    -z max-page-size=16
    -B ${CMAKE_BINARY_DIR}
    -T ${CMAKE_CURRENT_SOURCE_DIR}/platform/i686/i686.ld
    -specs=${CMAKE_CURRENT_SOURCE_DIR}/platform/i686/specs.txt
)

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
add_dependencies(platform_i686 crti crtn)

add_library(platform ALIAS platform_i686)

add_executable(boot
    boot/main.cpp
)
#target_compile_options(boot PRIVATE
#    -mgeneral-regs-only
#)
target_include_directories(boot PRIVATE
    .
    ${CMAKE_CURRENT_BINARY_DIR}/include
)
target_link_libraries(boot PRIVATE platform boot_std generic)
set_target_properties(boot PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/platform/i686/i686.ld)

add_custom_target("boot.bin" ALL ${CMAKE_OBJCOPY} -O binary boot boot.bin BYPRODUCTS boot.bin)
add_dependencies("boot.bin" boot)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/boot.bin DESTINATION boot)
