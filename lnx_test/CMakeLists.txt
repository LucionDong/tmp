cmake_minimum_required(VERSION 3.12)
project(modbus_tcp)
enable_language(C)
set(CMAKE_C_STANDARD 99)

find_package(Threads)

# add the path to the neuron header
include_directories(/usr/local/include /usr/local/include/neuron)
# add the path to the neuron library
link_directories(/usr/local/lib /usr/local/lib/neuron)

# add driver submodule
add_subdirectory(plugins/modbus-tcp)
