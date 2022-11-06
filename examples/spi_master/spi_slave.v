/******************************************************************************
File: spi_slave.v
Description: Simplistic SPI slave device model - Returns what has been received
in the previous transaction.
******************************************************************************/

/******************************************************************************
Includes and misc definitions
******************************************************************************/
`timescale 1us/10ps //Time scale definitions

/******************************************************************************
SPI master module
******************************************************************************/
module spi_slave(cs_b, mosi, miso, sclk);

    /**************************************************************************
    IOs
    **************************************************************************/
    input cs_b;     //Chip select signal, active low
    input mosi;     //Master out/Slave in signal
    input sclk;     //Clock
    output miso;    //Master in/Slave out signal

    /**************************************************************************
    Variables
    **************************************************************************/
    reg [7:0] tx_buffer [7:0];
    reg [7:0] rx_buffer [7:0];
    reg miso;
    wire sclk_or_cs_b;
    integer rx_byte_counter;
    integer rx_bit_counter;
    integer tx_byte_counter;
    integer tx_bit_counter;

    /**************************************************************************
    Tasks and functions
    **************************************************************************/
    task copy_rx_to_tx;
        integer i;
    begin
        for (i = 0; i < 8; i = i + 1) begin
            tx_buffer[i] = rx_buffer[i];
        end
    end
    endtask

    task rst_buffers;
        integer i;
    begin
        for (i = 0; i < 8; i = i + 1) begin
            rx_buffer[i] = 8'd0;
            tx_buffer[i] = 8'd0;
        end
        tx_buffer[0] = 8'h95;
    end
    endtask

    /* RX */
    always @(posedge sclk, posedge cs_b)
    begin
        if (cs_b == 1'b1) begin
            if (rx_byte_counter >= 7) copy_rx_to_tx();
            rx_bit_counter = 0;
            rx_byte_counter = 0;
        end else begin
            if (rx_byte_counter < 8)
                rx_buffer[7 - rx_byte_counter][7 - rx_bit_counter] = mosi;
            if (rx_bit_counter < 7) begin
                rx_bit_counter = rx_bit_counter + 1;
            end else begin
                rx_bit_counter = 0;
                rx_byte_counter = rx_byte_counter + 1;
            end
        end
    end

    /* TX */
    assign sclk_or_cs_b = sclk | cs_b;

    always @(negedge sclk_or_cs_b, posedge cs_b)
    begin
        if (cs_b == 1) begin
            miso = 1'bz;
            tx_bit_counter = 0;
            tx_byte_counter = 0;
        end else begin
            if (tx_byte_counter < 8)
                miso = tx_buffer[7 - tx_byte_counter][7 - tx_bit_counter];
            if (tx_bit_counter < 7) begin
                tx_bit_counter = tx_bit_counter + 1;
            end else begin
                tx_bit_counter = 0;
                tx_byte_counter = tx_byte_counter + 1;
            end
        end
    end

    initial begin
        miso = 1'bz;
        rst_buffers();
        rx_bit_counter = 0;
        rx_byte_counter = 0;
        tx_bit_counter = 0;
        tx_byte_counter = 0;
    end

endmodule
