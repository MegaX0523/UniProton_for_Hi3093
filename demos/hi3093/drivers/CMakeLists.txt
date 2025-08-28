cmake_minimum_required(VERSION 3.12)

add_definitions(-D__UNIPROTON__)

set(SRCS 
    gpio/gpio.c 
    gpio/gpio_pinmux_tbl.c
    spi/spi.c
    common/lp_subsys.c
    common/bm_common.c
    common/io_config.c
    common/reg_sysctrl.c
    osal/uniproton/osal_debug.c
    osal/uniproton/osal_interrupt.c
    osal/uniproton/osal_semaphore.c
)

# 创建对象库
add_library(drivers STATIC ${SRCS})

# 只添加 drivers 特有的头文件路径
target_include_directories(drivers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/gpio
    ${CMAKE_CURRENT_SOURCE_DIR}/i2c
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/cache
    ${CMAKE_CURRENT_SOURCE_DIR}/spi
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/osal/include
    ${CMAKE_CURRENT_SOURCE_DIR}/osal/include/module
)

# 通过接口库获取核心路径 (而不是直接包含)
target_link_libraries(drivers PUBLIC HI3093Core)
# target_link_libraries(HI3093Core PUBLIC drivers)
