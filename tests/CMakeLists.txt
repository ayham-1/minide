cmake_minimum_required(VERSION 3.25.3)

include(CTest)
include(CMakeParseArguments)
enable_testing()

set(CTEST_OUTPUT_ON_FAILURE TRUE)

add_compile_options(-frecord-gcc-switches)

find_program(MEMORYCHECK_COMMAND valgrind)
set(MEMORYCHECK_COMMAND_OPTIONS )
function(add_memcheck_test tname)
    add_test(memcheck_${tname} ${MEMORYCHECK_COMMAND} ${MEMORYCHECK_COMMAND_OPTIONS} ./${tname} )
endfunction(add_memcheck_test)

function(add_sanitize_test tname src)
    add_executable(sanitize_${tname} ${src})
    target_link_libraries(sanitize_${tname} PRIVATE min)
    add_test(sanitize_${tname} sanitize_${tname})
    target_compile_options(sanitize_${tname} PRIVATE -g3 -ggdb)
    target_link_options(sanitize_${tname} PRIVATE -static-libasan -O0
                -fno-stack-protector
                -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract
                -fsanitize=leak
                -fsanitize=undefined
                    -fsanitize=shift
                    -fsanitize=shift-exponent -fsanitize=shift-base -fsanitize=integer-divide-by-zero)
endfunction(add_sanitize_test)

function(gen_test name do_memtest do_sanitize)
    add_executable(test_${name} ${name}.c)
    target_link_libraries(test_${name} PRIVATE min)

    add_test(NAME test_${name}
             COMMAND $<TARGET_FILE:test_${name}>)
    set_property(TARGET test_${name} PROPERTY CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g3 -ggdb -fno-stack-protector -frecord-gcc-switches")

    if (do_memtest)
        add_memcheck_test(test_${name})
    endif()
    if (do_sanitize)
        add_sanitize_test(test_${name} ${name}.c)
    endif()

endfunction()

# u8string testing
gen_test(u8str_get_bytes_needed_for YES YES)
gen_test(u8str_get_wbyte_seq YES YES)
gen_test(u8str_clen YES YES)
gen_test(u8str_inc YES YES)
gen_test(u8str_dec YES YES)
gen_test(u8str_from_code_point YES YES)
gen_test(u8str_is_utf8_valid YES YES)

# path testing
gen_test(path_expand YES YES)

# hash table testing
gen_test(hash_table_insert_collision YES YES)
gen_test(hash_table_insert_unifont YES YES)
gen_test(hash_table_get YES YES)

# glyph caching
gen_test(glyph_cache YES NO)

# text renderer
gen_test(text_renderer YES NO)
