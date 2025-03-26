add_library(guish SHARED guish.c)

add_executable(guish-scanner scanner.c)
target_link_libraries(guish-scanner expat)

add_executable(guish-bin main.c guish parse.c process.c)
set_target_properties(guish-bin
	PROPERTIES OUTPUT_NAME guish)
target_link_libraries(guish-bin adt scallop-lang wayland-client)

install(TARGETS guish
	DESTINATION lib)
install(TARGETS guish-bin
	DESTINATION bin)
install(DIRECTORY guish
	DESTINATION include)
