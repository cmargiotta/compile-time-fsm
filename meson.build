project('gpt', 'cpp', default_options: ['cpp_std=c++20'])

if get_option('buildtype') == 'debug'
  add_project_arguments('-O0', '-g3', '-Wall',  language : 'cpp')
endif

add_project_link_arguments(['-lstdc++fs'], language: 'cpp')

cpp = meson.get_compiler('cpp')

catch2 = dependency('catch2')

subdir('src')
subdir('test')