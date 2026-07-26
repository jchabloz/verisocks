/*
Reset synchronizer
*/

module rst_sync #(
    parameter integer L = 2     // Shift register length
)(
    output  wire clk_arstb,     // Active-low reset, sync'ed
    input   wire arstb,         // Asynchronous, active-low reset
    input   wire clk            // Clock
);
    // Variables
    reg [L-1:0] shr;    // Shift register
  
    always @(posedge clk, negedge arstb) begin
        if (!arstb) shr <= {L{1'b0}};
        else shr <= {shr[L-2:0], 1'b1};
    end
    assign clk_arstb = shr[L-1];

endmodule

