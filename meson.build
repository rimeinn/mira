project('mira', 'cpp', version: '0.1.0', default_options: ['cpp_std=c++20'])

cmake = import('cmake')
run_command('bash', 'subprojects/setup.sh', check: true)
librime_options = cmake.subproject_options()
librime_options.add_cmake_defines({'BUILD_SHARED_LIBS': false,
                                   'BUILD_MERGED_PLUGINS': true,
                                   'ENABLE_EXTERNAL_PLUGINS': false})
librime_project = cmake.subproject('librime', options: librime_options)
librime = librime_project.dependency('rime-static').as_link_whole()

yamlcpp = dependency('yaml-cpp', version: '>=0.8')
argparse = dependency('argparse', version: '>=3.0')
lua = dependency('lua', version: '>=5.4')

subdir('include')
subdir('src')
