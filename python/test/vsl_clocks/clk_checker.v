/*
Clock checker - Calculates clock period and duty cycle
*/

`timescale 1ns/1ps

module clk_checker (
    input wire clk
);
    real time_posedge;
    real time_up;
    real time_period;
    real time_dc;

    always @(posedge clk) begin
        time_posedge <= $time;
        time_period <= $time - time_posedge;
        if ($time > time_posedge) time_dc <= time_up/($time - time_posedge);
    end

    always @(negedge clk) begin
        time_up <= $time - time_posedge;
    end

    initial begin
        time_posedge = 0;
        time_up = 0;
        time_period = 0;
        time_dc = 0.0;
    end

endmodule // clk_checker
// EOF
