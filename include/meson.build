conf_data = configuration_data()
conf_data.set('PROJECT_NAME', meson.project_name())
conf_data.set('PROJECT_VERSION', meson.project_version())
configure_file(input: 'config.h.in',
               output: 'config.h',
               configuration: conf_data)
incdir = include_directories('.')
