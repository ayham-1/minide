cmake_minimum_required(VERSION 3.25.3)

include(CTest)
include(CMakeParseArguments)
enable_testing()

add_compile_options(-frecord-gcc-switches)

find_program(MEMORYCHECK_COMMAND valgrind)
set(MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --leak-check=full")
function(add_memcheck_test tname)
    add_test(memcheck_${tname} ${MEMORYCHECK_COMMAND} ./${tname} ${MEMORYCHECK_COMMAND_OPTIONS}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIRECTORY})
endfunction(add_memcheck_test)

function(add_sanitize_test tname src)
    add_executable(sanitize_${tname} ${src})
    target_link_libraries(sanitize_${tname} PRIVATE min)
    add_test(sanitize_${tname} sanitize_${tname} WORKING_DIRECTORY ${CMAKE_SOURCE_DIRECTORY})
    target_compile_options(sanitize_${tname} PRIVATE -g3 -ggdb)
    target_link_options(sanitize_${tname} PRIVATE -static-libasan -O0
                -fno-stack-protector
                -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract
                -fsanitize=leak
                -fsanitize=undefined
                    -fsanitize=shift
                    -fsanitize=shift-exponent -fsanitize=shift-base -fsanitize=integer-divide-by-zero)
endfunction(add_sanitize_test)

function(gen_test name)
    add_executable(test_${name} ${name}.c)
    target_link_libraries(test_${name} PRIVATE min)

    add_test(NAME test_${name}
             COMMAND $<TARGET_FILE:test_${name}>)
    set_property(TARGET test_${name} PROPERTY CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g3 -ggdb -fno-stack-protector -frecord-gcc-switches")

    add_memcheck_test(test_${name})
    add_sanitize_test(test_${name} ${name}.c)
endfunction()

# u8string testing
gen_test(u8str_get_bytes_needed_for)
gen_test(u8str_get_wbyte_seq)
gen_test(u8str_clen)
gen_test(u8str_inc)
gen_test(u8str_dec)
gen_test(u8str_from_code_point)
gen_test(u8str_is_utf8_valid)

# path testing
gen_test(path_expand)

# hash table testing
gen_test(hash_table_insert_collision)
gen_test(hash_table_insert_unifont)
gen_test(hash_table_get)

# glyph caching
gen_test(glyph_cache)
