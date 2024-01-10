if not meson.is_subproject()
	catch2 = dependency('catch2')
	test_main = static_library('test_main', sources: ['catch_main.cpp'], dependencies: [catch2])

	utility_test = executable(
		'utility',
		'utility_test.cpp',
		link_with: [test_main],
		dependencies: [ctfsm, utility, catch2]
	)

	fsm_test = executable(
		'fsm',
		'fsm.cpp',
		link_with: [test_main],
		dependencies: [ctfsm, catch2]
	)

	test('utility test', utility_test)
	test('fsm test', fsm_test)
endif