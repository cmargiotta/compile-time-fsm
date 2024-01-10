project('ctfsm', 'cpp', default_options: ['cpp_std=c++20'])

if get_option('buildtype') == 'debug'
  add_project_arguments('-O0', '-g3', '-Wall',  language : 'cpp')
else
  add_project_arguments('-O3', '-Wall',  language : 'cpp')
endif

cpp = meson.get_compiler('cpp')

subdir('src')
subdir('test')

quom = find_program('quom', required: false)

if quom.found()
    quom_output = run_command(quom, '-I', meson.project_source_root() + '/src/', meson.project_source_root() + '/src/fsm/fsm.hpp', meson.project_build_root() + '/single-include/fsm.hpp', capture: true)

    if quom_output.returncode() != 0
        warning('Cannot build the single include header:')
        warning(quom_output.stderr())
    endif
endif
