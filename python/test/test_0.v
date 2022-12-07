`timescale 1us/1ps

`ifndef NUM_PORT
`define NUM_PORT 5100
`endif

module main;

parameter integer int_param = 598402;  //Used only for tests
parameter real fclk = 1.01; //MHz

reg clk;
reg [7:0] count;
reg [7:0] count_memory [0:15];
reg [3:0] mem_pointer;
event counter_end;

always @*
	clk <= #(1/fclk/2.0) ~clk;

always @(posedge(clk)) begin
	count <= count + 1;
	mem_pointer <= mem_pointer + 1;
	count_memory[mem_pointer] <= count;
	if (count == 8'd255)
		->counter_end;
end

initial begin

	$display("INFO [Top]: Initializing Verisocks");
	$verisocks_init(`NUM_PORT);

	clk = 1'b0;
	count = 8'd0;
	mem_pointer = 4'd0;

	#1000 $display("INFO [Top]: Simulation finished");
	$finish(0);
end

endmodule
