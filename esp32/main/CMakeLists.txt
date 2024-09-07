set(SRC "main.c" "wifi.c")

idf_component_register(SRCS ${SRC}
                       INCLUDE_DIRS ".")
