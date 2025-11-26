executable('mira', ['main.cpp', 'rime.cpp', 'output_collector.cpp'],
           include_directories: incdir,
           dependencies: [yamlcpp, argparse, lua, librime],
           cpp_args: ['-std=c++17'],
           install: true,
           link_args: '-no-pie')
