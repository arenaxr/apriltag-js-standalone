#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "test_str_json.h"

int main(void) {

    const struct CMUnitTest str_json_tests[] = {
        cmocka_unit_test(when_called_str_json_create_returns_a_valid_string),
        cmocka_unit_test(when_called_on_zero_len_str_json_create_returns_error),
        cmocka_unit_test(when_called_repeatedly_str_json_create_returns_error),
        cmocka_unit_test(when_called_str_json_destroy_destroys),
        cmocka_unit_test(when_called_str_clear_returns_an_empty_string),
        cmocka_unit_test(when_given_empty_inputs_str_json_concat_returns_a_valid_result),
        cmocka_unit_test(when_given_too_large_inputs_str_json_concat_returns_a_valid_result),
        cmocka_unit_test(when_concat_called_in_sequence_return_a_valid_result),
        cmocka_unit_test(when_clear_and_concat_called_in_sequence_return_a_valid_result),
        cmocka_unit_test(when_destroy_and_concat_called_in_sequence_return_a_valid_result),
        cmocka_unit_test(when_str_json_printf_called_returns_a_well_formatted_result),
        cmocka_unit_test(when_given_too_large_inputs_str_json_printf_returns_a_well_formatted_result)
    };

    /* Run the tests */
    return cmocka_run_group_tests(str_json_tests, NULL, NULL);
}
