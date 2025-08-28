set(SRC main.c ctltask.c filter.c gpio_set.c spi.c AD7606.c DAC8563.c)
add_library(rpmsg OBJECT ${SRC})
