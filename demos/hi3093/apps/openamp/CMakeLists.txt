file(GLOB SRC CONFIGURE_DEPENDS "*.c")
add_library(rpmsg OBJECT ${SRC})
