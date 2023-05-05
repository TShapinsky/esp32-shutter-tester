idf_component_register(SRCS "task_shutter_timer.c" "main.c" "measure.c"
                    INCLUDE_DIRS ".")
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
