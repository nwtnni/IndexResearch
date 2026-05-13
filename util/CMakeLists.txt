cmake_minimum_required(VERSION 3.10)
project(util)
set(CMAKE_CXX_STANDARD 17)

execute_process(COMMAND lscpu COMMAND grep avx512f OUTPUT_VARIABLE AVX512F_ENABLE)
execute_process(COMMAND lscpu COMMAND grep avx512bw OUTPUT_VARIABLE AVX512BW_ENABLE)
execute_process(COMMAND lscpu COMMAND grep avx OUTPUT_VARIABLE AVX_ENABLE)
execute_process(COMMAND lscpu COMMAND grep avx2 OUTPUT_VARIABLE AVX2_ENABLE)
execute_process(COMMAND lscpu COMMAND grep sse2 OUTPUT_VARIABLE SSE2_ENABLE)

if (AVX512F_ENABLE)
    message(STATUS "avx512f enable")
    add_definitions(-DAVX512F_ENABLE)
endif ()

if (AVX512BW_ENABLE)
    message(STATUS "avx512bw enable")
    add_definitions(-DAVX512BW_ENABLE)
endif ()

if (AVX_ENABLE)
    message(STATUS "avx enable")
    add_definitions(-DAVX_ENABLE)
endif ()

if (AVX2_ENABLE)
    message(STATUS "avx2 enable")
    add_definitions(-DAVX2_ENABLE)
endif ()

if (SSE2_ENABLE)
    message(STATUS "sse2 enable")
    add_definitions(-DSSE2_ENABLE)
endif ()

include_directories(./)

function(add_exe dir name)
    add_executable(util-${name} ${dir}/${name}.cpp)
    message(STATUS "add ${dir}/util_${name}")
endfunction()

function(add_dir dir)
    aux_source_directory(${dir} files)
    foreach (file ${files})
        get_filename_component(name ${file} NAME_WE)
        add_exe(${dir} ${name})
    endforeach ()
endfunction()


link_libraries(pthread numa jemalloc tbb pmem)
add_compile_options(-march=native)
add_dir(./example)
add_dir(./test)

target_link_libraries(util-bandwidth-test pmem)