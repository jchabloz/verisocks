/**
 * @file cunit_test.c
 * @author jchabloz
 * @brief Test suite for the verisocks module using CUnit
 * @version 0.1
 * @date 2022-08-25
 * 
 */

#include <stdlib.h>
#include <netdb.h>
#include <CUnit/Basic.h>
#include <CUnit/Automated.h>

/******************************************************************************
* Test suite - vs_msg module
******************************************************************************/
#include "test_vs_msg.c"

/******************************************************************************
* Test suite - vs_server module
******************************************************************************/
#include "test_vs_server.c"

/******************************************************************************
* Main
******************************************************************************/
int main(void)
{
    CU_pSuite pSuite = NULL;

    /* Initialize CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add vs_msg module test suite to registry */
    pSuite = CU_add_suite("Test suite vs_msg",
        init_suite_vs_msg, clean_suite_vs_msg);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add tests to suite*/
    if (
        (NULL == CU_add_test(pSuite,
            "Tests creating a JSON header for a JSON message content",
            test_vs_msg_create_header_json)) ||
        (NULL == CU_add_test(pSuite,
            "Tests creating a JSON header for a plain text message content",
            test_vs_msg_create_header_text)) ||
        (NULL == CU_add_test(pSuite,
            "Tests creating a JSON header for a binary message content",
            test_vs_msg_create_header_bin)) ||
        (NULL == CU_add_test(pSuite,
            "Tests creating a message with a JSON content",
            test_vs_msg_create_message_json)) ||
        (NULL == CU_add_test(pSuite,
            "Tests creating a message with a JSON content from a string",
            test_vs_msg_create_json_message_from_string)) ||
        (NULL == CU_add_test(pSuite,
            "Tests creating a message with a plain text content",
            test_vs_msg_create_message_text)) ||
        (NULL == CU_add_test(pSuite,
            "Tests creating a message with a binary content",
            test_vs_msg_create_message_bin)) ||
        (NULL == CU_add_test(pSuite,
            "Tests message read-write loopback",
            test_vs_msg_read_write_loopback))
    ) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add vs_server module test suite to registry */
    pSuite = CU_add_suite("Test suite vs_server",
        init_suite_vs_server, clean_suite_vs_server);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add tests to suite*/
    if (
        (NULL == CU_add_test(pSuite,
            "Tests vs_server_make_socket",
            test_vs_server_make_socket)) ||
        (NULL == CU_add_test(pSuite,
            "Tests vs_server_accept",
            test_vs_server_accept))
    ) {
        CU_cleanup_registry();
        return CU_get_error();
    }

/******************************************************************************
 * Run test suites
******************************************************************************/

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    /* Run all tests using the automated interface */
    CU_automated_run_tests();
    // CU_list_tests_to_file();

    /* Clean-up */
    CU_cleanup_registry();
    return CU_get_error();
}
