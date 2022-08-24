
set(BASE_HEADERS
  memory.h
)

set(BASE_OBJS
  memory.c
)

set(LIBTYPE STATIC)

set(LIBBASE base)
add_library(${LIBBASE} ${LIBTYPE} ${BASE_OBJS})
