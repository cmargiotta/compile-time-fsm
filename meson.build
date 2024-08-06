project('ctfsm', 'cpp', default_options: ['cpp_std=c++20'])

if get_option('buildtype') == 'debug'
  add_project_arguments('-O0', '-g3', '-Wall',  language : 'cpp')
else
  add_project_arguments('-O3', '-Wall',  language : 'cpp')
endif

add_project_link_arguments(['-lstdc++fs'], language: 'cpp')

cpp = meson.get_compiler('cpp')

catch2 = dependency('catch2')

subdir('src')
subdir('test')

quom = run_command('quom', '-I', meson.project_source_root() + '/src/', meson.project_source_root() + '/src/fsm/fsm.hpp', meson.project_source_root() + '/single-include/fsm.hpp', capture: true)

if quom.returncode() != 0
    warning('Cannot build the single include header:')
    warning(quom.stderr())
endif
