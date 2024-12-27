add_executable(guish guish.c parse-statement.c process.c)

target_link_libraries(guish adt scallop-lang)
