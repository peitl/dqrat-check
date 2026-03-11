#file(GLOB SOURCES *.cc)
set(sources
"constraint_DB.cc"
"dependency_manager_upure.cc"
"main.cc"
"parser.cc"
"dqbf.cc"
"dqrat_check.cc"
"watched_literal_propagator.cc"
)

add_executable(dqrat-check ${sources})
install(TARGETS dqrat-check)

#set_target_properties(dqrat-check PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
