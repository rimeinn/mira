executable('mira', ['main.cpp', 'rime.cpp', 'output_collector.cpp'],
           include_directories: incdir,
           dependencies: [yamlcpp, argparse, lua, librime, libunwind],
           cpp_args: ['-std=c++17'],
           install: true)
