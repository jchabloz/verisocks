/******************************************************************************
File: spi_master_tb.v
Description: SPI master testbench using Verisocks
******************************************************************************/

/*******************************************************************************
Includes and misc definitions
*******************************************************************************/
`timescale 1us/10ps //Time scale definitions

`ifndef VERILATOR
`ifndef VS_NUM_PORT
`define VS_NUM_PORT 5100
`endif

`ifndef VS_TIMEOUT
`define VS_TIMEOUT 120
`endif
`endif

/*******************************************************************************
Testbench
*******************************************************************************/
module spi_master_tb();

	real toto /*verilator public*/;
	integer tutu /*verilator public*/;
    reg titi /*verilator public*/;
    reg [6:0] tata [11:0] /*verilator public*/;

    wire cs_b, mosi, sclk, miso /*verilator public*/;

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
	
		toto = -1.623;
		tutu = 13;
        titi = 0;
		tata[0] = 7'd0;
		tata[1] = 7'd1;
		tata[2] = 7'd3;
		tata[3] = 7'd5;
		tata[4] = 7'd7;
		tata[5] = 7'd11;
		tata[6] = 7'd13;
		tata[7] = 7'd17;
		tata[8] = 7'd23;
		tata[9] = 7'd29;
		tata[10] = 7'd31;
		tata[11] = 7'd37;

        `ifdef DUMP_FILE
        $dumpfile(`DUMP_FILE);
        $dumpvars(0, spi_master_tb);
        `endif

        /* Launch Verisocks server after other initialization */
		`ifndef VERILATOR
        $verisocks_init(`VS_NUM_PORT, `VS_TIMEOUT);
		`endif

        /* Make sure that the simulation finishes after a while... */
        #10_000
        $finish(0);
    end

endmodule
