
set( MIDI_SOURCES
     midi.cpp
     driver.cpp
     file.cpp
     ensemble.cpp
     stream.cpp
     gmmidi.cpp
     gsmidi.cpp
     xgmidi.cpp
   )

set(SOURCES "")
foreach(src ${MIDI_SOURCES})
  set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${src};${SOURCES}")
endforeach()
