cmake_minimum_required(VERSION 3.25.3)

enable_testing()

function(gen_test name)
    add_executable(test_${name} ${name}.c)
    target_link_libraries(test_${name} PRIVATE min)
    add_test(NAME test_${name}
             COMMAND $<TARGET_FILE:test_${name}>)
    set_property(TARGET test_${name} PROPERTY CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g3 -ggdb -O0")
endfunction()

# u8string testing
gen_test(u8str_get_bytes_needed_for)
gen_test(u8str_get_wbyte_seq)
gen_test(u8str_clen)
gen_test(u8str_inc)
gen_test(u8str_dec)
