
set(BASE_HEADERS
  types.h
)

set(BASE_OBJS
  types.c
)

set(LIBTYPE STATIC)

set(LIBBASE base)
ADD_LIBRARY(${LIBBASE} ${LIBTYPE} ${BASE_OBJS})
