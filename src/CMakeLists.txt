
set( LIBBASE base)
set( DRIVER_OBJS
     driver.c
   )

set(LIBDRIVER driver)
add_library(${LIBDRIVER} STATIC ${DRIVER_OBJS})
target_link_libraries(${LIBDRIVER} ${LIBBASE})

function(CREATE_UTIL TEST_NAME)
    ADD_EXECUTABLE(${TEST_NAME} ${TEST_NAME}.c)
    TARGET_LINK_LIBRARIES(${TEST_NAME} ${LIBTHIRDPARTY} ${LIBDRIVER} ${AAX_LIBRARY} ${XML_LIBRARY} ${KERNEL_LIBRARY} ${EXTRA_LIBS})
    SET_TARGET_PROPERTIES(${TEST_NAME} PROPERTIES
            COMPILE_DEFINITIONS  "SRC_PATH=\"${PROJECT_SOURCE_DIR}/sounds\"")
endfunction()

function(CREATE_CPP_UTIL TEST_NAME)
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
    target_link_libraries(${TEST_NAME} ${LIBDRIVER} ${LIBMIDI} ${AAX_LIBRARY} ${XML_LIBRARY} ${KERNEL_LIBRARY} ${EXTRA_LIBS})
    set_target_properties(${TEST_NAME} PROPERTIES
            COMPILE_DEFINITIONS  "SRC_PATH=\"${PROJECT_SOURCE_DIR}/sounds\"")
endfunction()


CREATE_UTIL(drawbar2aaxs)
install(TARGETS drawbar2aaxs
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        COMPONENT Applications
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
CREATE_CPP_UTIL(aaxplaymidi)
install(TARGETS aaxplaymidi
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        COMPONENT Applications
)
