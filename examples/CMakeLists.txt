function(add_example example_name)
    file(GLOB_RECURSE EXAMPLE_SOURCES CONFIGURE_DEPENDS ${example_name}/*.cpp)
    add_executable(${example_name} ${EXAMPLE_SOURCES})
    target_link_libraries(${example_name} ${PROJECT_NAME})
    set_target_properties(${example_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/examples
    )
    message(STATUS "Added example: ${example_name}")
endfunction()

# Add all examples
file(GLOB EXAMPLE_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)
foreach(example_dir ${EXAMPLE_DIRS})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${example_dir})
        add_example(${example_dir})
    endif()
endforeach()