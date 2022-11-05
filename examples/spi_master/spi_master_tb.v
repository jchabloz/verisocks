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

    reg miso;
    wire cs_b, mosi, sclk;

    spi_master i_spi_master (
        .cs_b   (cs_b),
        .mosi   (mosi),
        .miso   (miso),
        .sclk   (sclk)
    );

    initial begin
        $dumpfile("spi_master_tb.fst");
        $dumpvars(0, spi_master_tb);
        miso = 1'b0;
        $verisocks_init(`VS_NUM_PORT, `VS_TIMEOUT);
        #10_000
        $finish(0);
    end

endmodule
