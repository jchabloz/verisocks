/******************************************************************************
File: spi_master.v
Description: SPI master emulation (behavioral model, non-synthesizable)
******************************************************************************/

/******************************************************************************
Includes and misc definitions
******************************************************************************/
`timescale 1us/10ps //Time scale definitions

/******************************************************************************
SPI master module
******************************************************************************/
module spi_master(cs_b, mosi, miso, sclk);

    /**************************************************************************
    IOs
    **************************************************************************/
    output cs_b;    //Chip select signal, active low
    output mosi;    //Master out/Slave in signal
    output sclk;    //Clock
    input  miso;    //Master in/Slave out signal

    /**************************************************************************
    Parameters
    **************************************************************************/
    parameter real
        spi_data_rate = 10.0,   //[Mbps] SPI data rate
        spi_clk_dc = 50.0,      //[%] SPI clock duty cycle
        spi_cs_tstart = 0.1,    //[us] Initial time after CS_B falling edge
        spi_cs_tstop = 0.1;     //[us] Final time to CS_B rising edge

    localparam
        CRC_DATA_WIDTH = 8,     //CRC input data width
        CRC_WIDTH = 8,          //CRC width
        CRC_POLY = 8'h2f,       //CRC polynomial
        CRC_SEED = 8'hff;       //CRC seed value

    /**************************************************************************
    Variables
    **************************************************************************/
    reg cs_b, mosi, sclk;
    reg [7:0] tx_buffer [6:0] /*verilator public*/;
    reg [7:0] rx_buffer [7:0] /*verilator public*/;
    reg [7:0] rx_crc;
    reg [255:0] ascii_cmd_id;
    reg [255:0] ascii_ret_id;
    reg rx_crc_error;
    event start_transaction /*verilator public*/;
    event end_transaction;
    integer transaction_counter;

    /**************************************************************************
    Function - Update CRC value
    Input(s):
        data: new data chunk
        crc: previous CRC value
    Output(s):
        nxt_crc: new CRC value
    **************************************************************************/
    function [CRC_WIDTH - 1:0] nxt_crc;
        //Inputs
        input   [CRC_DATA_WIDTH - 1:0]  data;
        input   [CRC_WIDTH - 1:0]       crc;
        //Variables
        integer i, j;
        reg     [CRC_WIDTH - 1:0]   c;
        reg     [CRC_WIDTH - 1:0]   c_tmp;
        //Function description
        begin
            c = crc;
            for (i = 0; i < CRC_DATA_WIDTH; i = i + 1) begin
                for (j = 0; j < CRC_WIDTH; j = j + 1) begin
                    c_tmp[j] = ((j > 0) ? c[j - 1] : 1'b0) ^
                               ((data[CRC_DATA_WIDTH - 1 - i] ^
                                c[CRC_WIDTH - 1]) & CRC_POLY[j]);
                end
                c = c_tmp;
            end
            nxt_crc = c;
        end
    endfunction

    /**************************************************************************
    Tasks and functions
    **************************************************************************/
    /**************************************************************************
    Task - Transmit an SPI byte
    Argument(s):
    - tx_byte: byte to transmit (MSB first)
    - rx_byte: receive byte buffer
    - bit_mask: 8-bit value enabling to inhibit the transmission/reception
                of one or several bits in the byte. If bit_mask[i] = 1, the
                i-th bit is not transmitted/received.
    **************************************************************************/
    task spi_transmit_byte;
        input [7:0] tx_byte;
        output [7:0] rx_byte;
        input [7:0] bit_mask;
        integer i;
        reg [7:0] rx_byte_tmp;
    begin
        rx_byte_tmp = 8'h00;
        for (i = 0; i < 8; i = i + 1) begin
            if (bit_mask[7 - i] == 0) begin
                mosi = tx_byte[7 - i];
                #(1/spi_data_rate*spi_clk_dc/100.0) sclk = 1'b1;
                rx_byte_tmp[7 - i] = miso;
                #(1/spi_data_rate*(1 - spi_clk_dc/100.0)) sclk = 1'b0;
            end
        end
        rx_byte = rx_byte_tmp;
    end
    endtask

    /**************************************************************************
    Task - Transmit/receive an SPI frame to RX and from TX buffers
    Transmitted from most significant byte to least significant byte
    Argument(s):
    - tx_mask: 8-bit value enabling to inhibit the transmission of one or
               several bytes in the frame. If tx_mask[i] = 1, the i-th byte
               is not transmitted.
    **************************************************************************/
    task spi_transmit_buffer_core;
        input [7:0] tx_mask;
        integer i;
        reg [7:0] crc;
    begin
        if (tx_mask[7] == 0) begin
            rx_crc_error = 1'b0;
            crc = CRC_SEED; //Initialize CRC with correct seed
            rx_crc = CRC_SEED;
        end
        for (i = 0; i < 7; i = i + 1) begin
            if (tx_mask[7 - i] == 0) begin
                crc = nxt_crc(tx_buffer[6 - i], crc);
                spi_transmit_byte(tx_buffer[6 - i], rx_buffer[7 - i], 0);
                rx_crc = nxt_crc(rx_buffer[7 - i], rx_crc);
            end
        end
        if (tx_mask[0] == 0) begin
            spi_transmit_byte(crc, rx_buffer[0], 0);
            rx_crc = nxt_crc(rx_buffer[0], rx_crc);
            rx_crc_error = (rx_crc == 8'd0) ? 1'b0 : 1'b1;
        end
    end
    endtask

    task spi_transmit_buffer;
        input [7:0] tx_mask;
    begin
        cs_b = 1'b0;
        #(0.0) sclk = 1'b0;
        #(spi_cs_tstart);
        spi_transmit_buffer_core(tx_mask);
        #(spi_cs_tstop) cs_b = 1'b1;
        transaction_counter = transaction_counter + 1;
        ->end_transaction;
    end
    endtask

    task init_tx_buffer;
        integer i;
    begin
        for (i=0; i < 7; i = i + 1) begin
            tx_buffer[i] = 8'd0;
        end
    end
    endtask

    /**************************************************************************
    Hook for Verisocks to trigger task execution using a named event
    **************************************************************************/
    always @(start_transaction)
        spi_transmit_buffer(8'd0);

    /**************************************************************************
    Hook for Verilator - A public task (as declared using the verilator public
    comment is used here. It could also have been exported as a DPI method,
    which would make it also compatible with other simulators, however, passing
    arguments gets more complex. Another alternative would have been to declare
    the start_transaction and tx_buffer variables as public (I tested, this
    works fine), but such a task makes for a better, more readable,
    encapsulation.
    **************************************************************************/
    `ifdef VERILATOR
    task trigger_transaction;
        /*verilator public*/
        input [7:0] tx_val [6:0];
        integer i;
    begin
        for (i=0; i < 7; i = i + 1) begin
            tx_buffer[i] = tx_val[i];
        end
        ->start_transaction;
    end
    endtask
    `endif

    /**************************************************************************
    Initial
    **************************************************************************/
    initial begin
        cs_b = 1'b1;
        sclk = 1'b0;
        mosi = 1'b0;
        ascii_cmd_id = "N/A";
        ascii_ret_id = "N/A";
        rx_crc_error = 1'b0;
        transaction_counter = 0;
        init_tx_buffer();
    end

endmodule

