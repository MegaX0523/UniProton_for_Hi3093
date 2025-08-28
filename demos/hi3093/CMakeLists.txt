cmake_minimum_required(VERSION 3.12)

set(HOME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)

if (${APP} STREQUAL "cxxTest" OR ${APP} STREQUAL "eigenTest")
    set(CMAKE_C_COMPILER ${CXX_TOOLCHAIN_PATH}/bin/aarch64-openeuler-linux-gcc)
    set(CMAKE_CXX_COMPILER ${CXX_TOOLCHAIN_PATH}/bin/aarch64-openeuler-linux-g++)
    set(CMAKE_ASM_COMPILER ${CXX_TOOLCHAIN_PATH}/bin/aarch64-openeuler-linux-gcc)
    set(CMAKE_LINKER ${CXX_TOOLCHAIN_PATH}/bin/aarch64-openeuler-linux-ld)
else()
    set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-gcc)
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-g++)
    set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-gcc)
    set(CMAKE_LINKER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-ld)
endif()

project(exmaples LANGUAGES C CXX ASM)

set(CC_OPTION "-g -march=armv8.2-a -nostdlib -nostdinc -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -nostartfiles -funwind-tables")
set(AS_OPTION "-g -march=armv8.2-a -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -nostartfiles -mgeneral-regs-only -DENV_EL1")
set(LD_OPTION "-static -no-pie -Wl,--wrap=memset -Wl,--wrap=memcpy -Wl,-gc-sections -Wl,--eh-frame-hdr")
set(CMAKE_C_FLAGS "${CC_OPTION} ${POSIX_OPTION}")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}  -nostdinc++")
set(CMAKE_ASM_FLAGS "${AS_OPTION} ${POSIX_OPTION}")
set(CMAKE_LINK_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/hi3093.ld")
set(CMAKE_EXE_LINKER_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/hi3093.ld")
set(CMAKE_CXX_STANDARD 17)

include(${HOME_PATH}/cmake/functions/uniproton_functions.cmake)
import_kconfig(${HOME_PATH}/build/uniproton_config/config_armv8_hi3093/defconfig)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/libc/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/libc/include/bits
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/hi3093
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/uart
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/platform
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/src/real_time/drivers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/platform/osal/include
    ${HOME_PATH}/src/utility/lib/include
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/openamp
    ${CMAKE_CURRENT_SOURCE_DIR}/build/open-amp/output/usr/local/include
    ${CMAKE_CURRENT_SOURCE_DIR}/build/libmetal/output/usr/local/include
    ${CMAKE_CURRENT_SOURCE_DIR}/component/libcxx/include
    ${HOME_PATH}/output/libc/include
    ${CMAKE_CURRENT_SOURCE_DIR}/component/boost_1_54_0
    ${CMAKE_CURRENT_SOURCE_DIR}/component/eigen-3.4.0
    ${HOME_PATH}/output/soem/include
    ${HOME_PATH}/src/component/mica
    ${HOME_PATH}/output/libc/include
    ${HOME_PATH}/src/shell/adapter/include
    ${HOME_PATH}/src/shell/full/include
    ${HOME_PATH}/src/component/mica
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/include
    ${HOME_PATH}/src/arch/include
    ${HOME_PATH}/src/core/kernel/irq
    ${HOME_PATH}/src/core/kernel/include
)

add_library(HI3093Core INTERFACE)
target_include_directories(HI3093Core INTERFACE
    ${HOME_PATH}/src/core/ipc/include
    ${HOME_PATH}/src/om/include
    ${HOME_PATH}/src/mem/include
    ${HOME_PATH}/src/include/posix
    ${HOME_PATH}/src/osal/posix/include
    ${HOME_PATH}/src/utility/lib/include
)

add_subdirectory(${HOME_PATH}/src/component/proxy proxy)
add_subdirectory(bsp)
add_subdirectory(config)
add_subdirectory(apps)
add_subdirectory(drivers)

add_library(HI3093 INTERFACE)
target_link_libraries(HI3093 INTERFACE
    HI3093Core
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/libHI3093.a
)

if (${CONFIG_OS_OPTION_OPENAMP})
    add_subdirectory(${HOME_PATH}/src/component/mica mica)
    list(APPEND OBJS $<TARGET_OBJECTS:mica>)
endif()

    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart>)
    if (${CONFIG_LOSCFG_SHELL_TEST})
        add_subdirectory(${HOME_PATH}/testsuites/cyclictest tmp)
        add_subdirectory(${HOME_PATH}/testsuites/rhealstone tmp1)
        list(APPEND OBJS $<TARGET_OBJECTS:cyclicTest> $<TARGET_OBJECTS:rhealstoneTest>)
    endif()
    add_executable(${APP} ${OBJS})
    target_link_libraries(${APP} PRIVATE 
        HI3093  # 替换原有的链接方式
        drivers
        -Wl,--start-group
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/libCortexMXsec_c.lib"
        -Wl,--no-whole-archive
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/libmetal.a"
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/libopen_amp.a"
        -Wl,--end-group
        )

