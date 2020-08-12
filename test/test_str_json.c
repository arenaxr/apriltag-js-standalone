#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "str_json.h"

void when_called_str_json_create_returns_a_valid_string()
{
    t_str_json str_json = STR_JSON_INITIALIZER;
    int r = str_json_create(&str_json, 1000);

    assert_int_equal(r, 0);
    assert_int_equal(str_json.alloc_size, 1000);
    assert_int_equal(str_json.len, 0);

    str_json_destroy(&str_json);
}

void when_called_on_zero_len_str_json_create_returns_error()
{
    t_str_json str_json = STR_JSON_INITIALIZER;
    int r = str_json_create(&str_json, 0);

    assert_int_equal(r, -1);

    str_json_destroy(&str_json);
}

void when_called_repeatedly_str_json_create_returns_error()
{
    t_str_json str_json = STR_JSON_INITIALIZER;
    int r = str_json_create(&str_json, 1000);

    assert_int_equal(r, 0);
    assert_int_equal(str_json.alloc_size, 1000);
    assert_int_equal(str_json.len, 0);
    assert_non_null(str_json.str);
    char *ptr = str_json.str;

    r = str_json_create(&str_json, 500);
    assert_int_equal(r, -1);
    assert_int_equal(str_json.alloc_size, 1000);
    assert_int_equal(str_json.len, 0);
    assert_ptr_equal(ptr, str_json.str);

    str_json_destroy(&str_json);
}

void when_called_str_json_destroy_destroys()
{
    t_str_json str_json = STR_JSON_INITIALIZER;
    str_json_create(&str_json, 1000);
    int r = str_json_destroy(&str_json);
    assert_int_equal(r, 0);
    assert_int_equal(str_json.alloc_size, 0);
    assert_int_equal(str_json.len, 0);
    assert_null(str_json.str);

    // try again
    r = str_json_destroy(&str_json);
    assert_int_equal(r, -1);
}

void when_called_str_clear_returns_an_empty_string()
{
    t_str_json str_json = STR_JSON_INITIALIZER;
    str_json_create(&str_json, 1000);

    str_json_concat( &str_json, "A test string.");

    str_json_clear(&str_json);
    assert_int_equal(str_json.alloc_size, 1000);
    assert_int_equal(str_json.len, 0);
    assert_non_null(str_json.str);

    str_json_destroy(&str_json);
}

void when_given_empty_inputs_str_json_concat_returns_a_valid_result()
{
    t_str_json str_json = STR_JSON_INITIALIZER;
    str_json_create(&str_json, 1000);

    int r = str_json_concat( &str_json, "");
    assert_int_equal(r, 0);
    assert_int_equal(str_json.alloc_size, 1000);
    assert_int_equal(str_json.len, 0);
    assert_string_equal(str_json.str, "");

    r = str_json_concat( &str_json, "Hello");
    assert_int_equal(r, 5);
    assert_int_equal(str_json.alloc_size, 1000);
    assert_int_equal(str_json.len, 5);
    assert_string_equal(str_json.str, "Hello");

    str_json_destroy(&str_json);
}

void when_given_too_large_inputs_str_json_concat_returns_a_valid_result()
{
    t_str_json str_json = STR_JSON_INITIALIZER, str_json1 = STR_JSON_INITIALIZER;
    str_json_create(&str_json, 10);

    int r = str_json_concat( &str_json, "A large input!");
    assert_int_equal(r, 10);
    assert_int_equal(str_json.alloc_size, 10);
    assert_int_equal(str_json.len, 10);
    assert_string_equal(str_json.str, "A large in");

    r = str_json_concat( &str_json, "Lets try again!");
    assert_int_equal(r, 10);
    assert_int_equal(str_json.alloc_size, 10);
    assert_int_equal(str_json.len, 10);
    assert_string_equal(str_json.str, "A large in");

    str_json_create(&str_json1, 10);

    r = str_json_concat( &str_json1, "Exactly 10");
    assert_int_equal(r, 10);
    assert_int_equal(str_json1.alloc_size, 10);
    assert_int_equal(str_json1.len, 10);
    assert_string_equal(str_json1.str, "Exactly 10");

    str_json_destroy(&str_json);
    str_json_destroy(&str_json1);
}

void when_concat_called_in_sequence_return_a_valid_result()
{
    t_str_json str_json = STR_JSON_INITIALIZER;
    str_json_create(&str_json, 10);

    str_json_concat( &str_json, "One");
    assert_string_equal(str_json.str, "One");
    str_json_concat( &str_json, "Two");
    assert_string_equal(str_json.str, "OneTwo");
    str_json_concat( &str_json, "Three");
    assert_string_equal(str_json.str, "OneTwoThre");
    assert_int_equal(str_json.alloc_size, 10);
    assert_int_equal(str_json.len, 10);

    str_json_destroy(&str_json);
}

void when_clear_and_concat_called_in_sequence_return_a_valid_result()
{
    t_str_json str_json = STR_JSON_INITIALIZER;
    str_json_create(&str_json, 10);

    str_json_concat( &str_json, "One");
    str_json_concat( &str_json, "Two");
    str_json_concat( &str_json, "Three");
    assert_string_equal(str_json.str, "OneTwoThre");

    str_json_clear(&str_json);
    assert_int_equal(str_json.alloc_size, 10);
    assert_int_equal(str_json.len, 0);
    assert_string_equal(str_json.str, "");

    str_json_concat( &str_json, "One");
    str_json_concat( &str_json, "Two");
    str_json_concat( &str_json, "Three");
    assert_string_equal(str_json.str, "OneTwoThre");
    assert_int_equal(str_json.alloc_size, 10);
    assert_int_equal(str_json.len, 10);

    str_json_clear(&str_json);
    assert_int_equal(str_json.alloc_size, 10);
    assert_int_equal(str_json.len, 0);
    assert_string_equal(str_json.str, "");

    str_json_destroy(&str_json);
}

void when_destroy_and_concat_called_in_sequence_return_a_valid_result()
{
    t_str_json str_json = STR_JSON_INITIALIZER;
    str_json_create(&str_json, 10);

    str_json_concat( &str_json, "One");
    str_json_concat( &str_json, "Two");
    str_json_concat( &str_json, "Three");
    assert_string_equal(str_json.str, "OneTwoThre");

    str_json_destroy(&str_json);
    str_json_create(&str_json, 20);
    assert_int_equal(str_json.alloc_size, 20);
    assert_int_equal(str_json.len, 0);
    assert_string_equal(str_json.str, "");

    str_json_concat( &str_json, "One");
    str_json_concat( &str_json, "Two");
    str_json_concat( &str_json, "Three");
    assert_string_equal(str_json.str, "OneTwoThree");
    assert_int_equal(str_json.alloc_size, 20);
    assert_int_equal(str_json.len, 11);

    str_json_destroy(&str_json);
    str_json_create(&str_json, 30);
    assert_int_equal(str_json.alloc_size, 30);
    assert_int_equal(str_json.len, 0);
    assert_string_equal(str_json.str, "");

    str_json_destroy(&str_json);
}

void when_str_json_printf_called_returns_a_well_formatted_result()
{
    t_str_json str_json = STR_JSON_INITIALIZER;
    str_json_create(&str_json, 100);

    str_json_printf(&str_json, "A value: %d", 5);
    assert_string_equal(str_json.str, "A value: 5");
    assert_int_equal(str_json.alloc_size, 100);
    assert_int_equal(str_json.len, 10);

    str_json_destroy(&str_json);
}

void when_given_too_large_inputs_str_json_printf_returns_a_well_formatted_result()
{
    t_str_json str_json = STR_JSON_INITIALIZER;
    str_json_create(&str_json, 30);

    str_json_printf(&str_json, "A value: %s", "this is a long string");
    assert_string_equal(str_json.str, "A value: this is a long strin");

    str_json_destroy(&str_json);
}
