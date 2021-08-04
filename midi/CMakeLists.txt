
set( MIDI_OBJS
     driver.cpp
     file.cpp
     instrument.cpp
     stream.cpp
     gmmidi.cpp
     gsmidi.cpp
     xgmidi.cpp
   )

set(LIBTYPE STATIC)

set(LIBMIDI midi)
ADD_LIBRARY(${LIBMIDI} ${LIBTYPE} ${MIDI_OBJS})
