cmake_minimum_required(VERSION 3.21)

set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -nodefaultlibs -Wa,--divide")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -nodefaultlibs -ffreestanding -Wall -Wextra -pedantic -pedantic-errors")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nodefaultlibs -ffreestanding -Wall -Wextra -pedantic -pedantic-errors -fno-exceptions") # -fno-rtti

project(boot LANGUAGES ASM C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)
