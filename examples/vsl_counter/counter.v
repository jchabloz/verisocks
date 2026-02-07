
module counter #(
	parameter integer LENGTH = 10	// Counter register length
) (
	input  wire arst_b,				// Active-low, asynchronous reset
	input  wire clk,				// Clock
	output wire [LENGTH-1:0] count	// Counter value
);

	reg [LENGTH-1:0] counter;

	always @(posedge clk, negedge arst_b)
	begin
		if (!arst_b)
			counter <= {LENGTH{1'b0}};
		else
			counter <= counter + 1;
	end
	assign count = counter;

	`ifdef DUMP_FILE
	initial
	begin
		$dumpfile(`DUMP_FILE);
        $dumpvars(0, counter);
	end
	`endif

endmodule
// EOF
