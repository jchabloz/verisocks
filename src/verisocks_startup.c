/**
 * @file verisocks_startup.c
 * @author jchabloz
 * @brief VPI boostrap
 * @version 0.1
 * @date 2022-09-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "verisocks.h"
#include "vpi_config.h"

void (*vlog_startup_routines[])() =
{
    verisocks_register_tf,
    0
};
