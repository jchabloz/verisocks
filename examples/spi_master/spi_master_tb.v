/******************************************************************************
File: spi_master_tb.v
Description: SPI master testbench using Verisocks
******************************************************************************/

/*******************************************************************************
Includes and misc definitions
*******************************************************************************/
`timescale 1us/10ps //Time scale definitions

`define VS_NUM_PORT 5100
`define VS_TIMEOUT 120

/*******************************************************************************
Testbench
*******************************************************************************/
module spi_master_tb();

    wire cs_b, mosi, sclk, miso;

    /* SPI master - unit under test */
    spi_master i_spi_master (
        .cs_b   (cs_b),
        .mosi   (mosi),
        .miso   (miso),
        .sclk   (sclk)
    );

    /* Simplistic slave implementation */
    spi_slave i_spi_slave (
        .cs_b   (cs_b),
        .mosi   (mosi),
        .miso   (miso),
        .sclk   (sclk)
    );

    /* Initial loop */
    initial begin
        $dumpfile("spi_master_tb.fst");
        $dumpvars(0, spi_master_tb);

        /* Launch Verisocks server after other initialization */
        $verisocks_init(`VS_NUM_PORT, `VS_TIMEOUT);

        /* Make sure that the simulation finishes after a while... */
        #10_000
        $finish(0);
    end

endmodule
