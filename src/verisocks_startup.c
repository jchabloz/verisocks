/**
 * @file verisocks_startup.c
 * @author jchabloz (jeremie.chabloz@a3.epfl.ch)
 * @brief VPI boostrap
 * @version 0.1
 * @date 2022-09-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "verisocks.h"
#include <iverilog/vpi_user.h>

void (*vlog_startup_routines[])() =
{
    verisocks_register_tf,
    0
};
