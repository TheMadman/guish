add_executable(guish guish.c parse.c process.c)

target_link_libraries(guish adt scallop-lang wayland-client)
