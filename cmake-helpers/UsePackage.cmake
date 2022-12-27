function(UsePackage LIB_NAME LIB_FOLDER PRIVATE_LIBS PUBLIC_LIBS HAS_TESTS SHOULD_DEBUG)
    set(LIB_NAME_TEST ${LIB_NAME}Test)

    if (SHOULD_DEBUG)
        message(STATUS "LIB_NAME: ${LIB_NAME}")
        message(STATUS "LIB_FOLDER: ${LIB_FOLDER}")
        message(STATUS "LIB_NAME_TEST: ${LIB_NAME_TEST}")
        message(STATUS "PRIVATE_LIBS: ${PRIVATE_LIBS}")
        message(STATUS "PUBLIC_LIBS: ${PUBLIC_LIBS}")
        message(STATUS "HAS_TESTS: ${HAS_TESTS}")
    endif ()

    aux_source_directory(${LIB_FOLDER} SOURCES)
    list(FILTER SOURCES INCLUDE REGEX ".*test.cpp$")

    add_library(${LIB_NAME} STATIC ${SOURCES})
    target_include_directories(${LIB_NAME} INTERFACE .)

    if (PRIVATE_LIBS)
        foreach (PRIVATE_LIB ${PRIVATE_LIBS})
            target_link_libraries(${LIB_NAME} PRIVATE ${PRIVATE_LIB})
        endforeach ()
    endif ()

    if (PUBLIC_LIBS)
        foreach (PUBLIC_LIB ${PUBLIC_LIBS})
            target_link_libraries(${LIB_NAME} PUBLIC ${PUBLIC_LIB})
        endforeach ()
    endif ()

    if (HAS_TESTS)
        add_test(AllTests${LIB_NAME} ${LIB_NAME})

        aux_source_directory(${LIB_FOLDER} SOURCES_TEST)
        add_executable(${LIB_NAME_TEST} ${SOURCES_TEST})

        if (PRIVATE_LIBS)
            foreach (PRIVATE_LIB ${PRIVATE_LIBS})
                target_link_libraries(${LIB_NAME_TEST} PRIVATE ${PRIVATE_LIB})
            endforeach ()
        endif ()

        if (PUBLIC_LIBS)
            foreach (PUBLIC_LIB ${PUBLIC_LIBS})
                target_link_libraries(${LIB_NAME_TEST} PUBLIC ${PUBLIC_LIB})
            endforeach ()
        endif ()

        target_link_libraries(${LIB_NAME_TEST} PRIVATE
                gtest
                gmock
                gmock_main)
    endif ()

    set(LIB_NAME ${LIB_NAME} PARENT_SCOPE)
    set(LIB_FOLDER ${LIB_FOLDER} PARENT_SCOPE)
    set(LIB_NAME_TEST ${LIB_NAME}Test PARENT_SCOPE)
endfunction()
