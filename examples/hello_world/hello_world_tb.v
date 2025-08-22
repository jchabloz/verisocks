/******************************************************************************
File: hello_world_tb.v
Description: Hello world example testbench for Verisocks
******************************************************************************/

/*******************************************************************************
Includes and misc definitions
*******************************************************************************/
`timescale 1us/10ps //Time scale definitions

`ifndef VS_NUM_PORT
`define VS_NUM_PORT 5100
`endif

`ifndef VS_TIMEOUT
`define VS_TIMEOUT 120
`endif

/*******************************************************************************
Testbench
*******************************************************************************/
module hello_world_tb();

    initial begin

		`ifndef VERILATOR
        /* Launch Verisocks server after other initialization */
        $verisocks_init(`VS_NUM_PORT, `VS_TIMEOUT);
		`endif

        /* Make sure that the simulation finishes after a while... */
        #1000
        $finish(0);

    end

endmodule
