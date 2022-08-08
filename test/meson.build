# Compile time test, no need to exec
test_main = static_library('test_main', sources: ['catch_main.cpp'], dependencies: [catch2])

utility_test = static_library(
	'utility_test', 
	sources: ['utility_test.cpp'], 
	dependencies: [utility]
) 

fsm_test = executable(
	'fsm', 
	'fsm.cpp', 
	link_with: [test_main], 
	dependencies: [fsm, catch2]
)


test('fsm test', fsm_test)