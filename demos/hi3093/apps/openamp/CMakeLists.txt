set(SRC main.c gpio_set.c hi_spi.c AD7606.c DAC8563.c ctltask.c filter.c)
add_library(rpmsg OBJECT ${SRC})
