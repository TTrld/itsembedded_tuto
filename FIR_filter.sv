module FIR_filter #(
    parameter int taps        = 32,
    parameter int num_bits    = 8,
    parameter int input_size  = 8,
    parameter int output_size = (2 * num_bits) + 1
)(
    input  logic                      clock,
    input  logic                      reset,
    input  logic [input_size-1:0]     Data_In,
    output logic [output_size-1:0]    Data_Out
);

    // -----------------------------
    // FILTER COEFFICIENTS
    // -----------------------------
    localparam logic [num_bits-1:0] coeffs [0:taps-1] = '{
        8'b0000_0011, 8'b0000_0010, 8'b0000_0001, 8'b0000_0000,
        8'b0000_0000, 8'b0000_0000, 8'b0000_0000, 8'b0000_0000,
        8'b0000_0000, 8'b0000_0000, 8'b0000_0100, 8'b0000_1100,
        8'b0001_0101, 8'b0001_1110, 8'b0010_0101, 8'b0010_1001,
        8'b0010_1001, 8'b0010_0101, 8'b0001_1110, 8'b0001_0101,
        8'b0000_1100, 8'b0000_0100, 8'b0000_0000, 8'b0000_0000,
        8'b0000_0000, 8'b0000_0000, 8'b0000_0000, 8'b0000_0000,
        8'b0000_0000, 8'b0000_0001, 8'b0000_0010, 8'b0000_0011
    };

    // -----------------------------
    // Delay line register
    // -----------------------------
    logic [input_size-1:0] FIR[taps-1:0];

    // -----------------------------
    // Output Accumulation
    // -----------------------------
    logic [output_size-1:0] acc;
    integer j;

    always_comb begin
        acc = 0;
        for (j = 0; j < taps; j++) begin
            acc += coeffs[j] * FIR[j];
        end
    end

    assign Data_Out = acc;

    // -----------------------------
    // Shift Register Logic
    // -----------------------------
    integer i;
    always_ff @(posedge clock or posedge reset) begin
        if (reset) begin
            for (i = 0; i < taps; i++) begin
                FIR[i] <= 0;
            end
        end else begin
            for (i = taps-1; i > 0; i--) begin
                FIR[i] <= FIR[i-1];
            end
            FIR[0] <= Data_In;
        end
    end

endmodule
