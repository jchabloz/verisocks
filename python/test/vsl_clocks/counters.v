/*
This module has two clock domains, clk1 and clk2, which are considered as
completely asynchronous and with any possible frequencies relationship between
them (clk1 can be much faster than clk2 and vice versa).

A binary counter is incremented in clk1 domain and we would like to also make
its value available in the clk2 domain as often as possible while following
best practices in terms of clock domain crossing principles.
*/

`timescale 1ns/1ps

module counters #(
    parameter integer W = 10
)(
    input wire arstb,               // Async reset, active low
    input wire clk1,
    input wire clk2,
    output reg [W-1:0] clk1_count,
    output reg [W-1:0] clk2_count
);

    localparam IDLE     = 1'b0;
    localparam READING  = 1'b1;

    // Variables
    wire            clk1_arstb;
    wire            clk2_arstb;
    reg             state;
    reg             read_toggle;
    reg  [W-1:0]    count_buffer;
    reg  [2:0]      sr_read_toggle;
    reg  [2:0]      sr_read_ack;
    wire            read_req;
    wire            read_ack;

    // Synchronize the asynchronous reset in both clock domains
    rst_sync #(.L(2)) i_rst_sync1 (
        // Output
        .clk_arstb  (clk1_arstb),
        // Inputs
        .arstb      (arstb),
        .clk        (clk1)
    );

    rst_sync #(.L(2)) i_rst_sync2 (
        // Output
        .clk_arstb  (clk2_arstb),
        // Inputs
        .arstb      (arstb),
        .clk        (clk2)
    );

    clk_checker clk1_checker (clk1);
    clk_checker clk2_checker (clk2);

    // Binary counter in clk1 domain
    always @(posedge clk1, negedge clk1_arstb) begin
        if (!clk1_arstb) clk1_count <= 'd0;
        else clk1_count <= clk1_count + 'd1;
    end

    // Binary counter in clk2 domain
    always @(posedge clk2, negedge clk2_arstb) begin
        if (!clk2_arstb) clk2_count <= 'd0;
        else clk2_count <= clk2_count + 'd1;
    end

    initial begin
        $dumpfile("dump.fst");
        $dumpvars(0);
    end

endmodule

