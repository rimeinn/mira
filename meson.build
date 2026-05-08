project('mira', 'cpp',
        version: '0.3.0',
        default_options: ['cpp_std=c++20'],
        meson_version: '>=1.1')

lua_version = get_option('lua_version')
librime_commit = get_option('librime_commit')

if get_option('prefer_static')
  cmake = import('cmake')
  run_command('bash', 'subprojects/setup.sh', librime_commit, check: true)
  librime_options = cmake.subproject_options()
  librime_options.add_cmake_defines({'CMAKE_FIND_LIBRARY_SUFFIXES': '.a',
                                     'BUILD_SHARED_LIBS': false,
                                     'BUILD_STATIC': true,
                                     'BUILD_MERGED_PLUGINS': true,
                                     'ENABLE_EXTERNAL_PLUGINS': false,
                                     'LUA_VERSION': 'lua' + lua_version.replace('.', '')})
  librime_project = cmake.subproject('librime', options: librime_options)
  librime = librime_project.dependency('rime-static').as_link_whole()

  # link with deps bundled with librime
  yamlcpp = declare_dependency()
else
  librime = dependency('rime', version: '>=1.10')
  yamlcpp = dependency('yaml-cpp', version: '>=0.8')
endif

argparse = dependency('argparse', version: '>=3.0')

lua = dependency('lua' + lua_version, static: get_option('prefer_static'))

subdir('include')
subdir('src')
