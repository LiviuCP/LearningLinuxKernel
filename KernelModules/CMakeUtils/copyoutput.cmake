function (copy_file_to_consolidated_output OUTPUT_FILE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE} ${KERNEL_MODULES_CONSOLIDATED_OUTPUT_DIR}/${OUTPUT_FILE})
endfunction()
