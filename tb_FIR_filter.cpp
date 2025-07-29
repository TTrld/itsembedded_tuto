#include <iostream>
#include <deque>
#include <cstdlib>
#include <ctime>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "VFIR_filter.h"

#define MAX_SIM_TIME      600
#define VERIF_START_TIME  7
vluint64_t sim_time = 0;

// Globals for sine wave generation
const double PI = 3.141592653589793;
double sine_freq = 0.05; // Will be randomized
int sine_phase = 0;
int sample_rate = 1;     // 1 sample per tick (can adjust as needed)

class FilterInTx {
public:
    uint8_t Data_In;
};

class FilterOutTx {
public:
    uint32_t Data_Out;
};

class FilterScb {
private:
    std::deque<FilterInTx*> history;
public:
    void writeIn(FilterInTx* tx) {
        history.push_front(tx);
        if (history.size() > 32)
            history.pop_back();
    }

    void writeOut(FilterOutTx* tx) {
        if (history.size() < 32) {
            delete tx;
            return; // not enough history yet
        }

        // Use coefficients from Verilog
        uint8_t h[] = {
            0x03, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x04, 0x0C, 0x15, 0x1E, 0x25, 0x29,
            0x29, 0x25, 0x1E, 0x15, 0x0C, 0x04, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03
        };

        uint32_t expected = 0;
        for (int i = 0; i < 32; ++i) {
            expected += h[i] * history[i]->Data_In;
        }

        if (expected != tx->Data_Out) {
            std::cout << "[ERROR] @ " << sim_time << ": expected "
                      << expected << ", got " << tx->Data_Out << std::endl;
        }

        delete tx;
    }

    ~FilterScb() {
        for (auto* tx : history) delete tx;
    }
};

class FilterDrv {
private:
    VFIR_filter* dut;
public:
    FilterDrv(VFIR_filter* dut): dut(dut) {}

    void drive(FilterInTx* tx) {
        if (tx) {
            dut->Data_In = tx->Data_In;
        }
        delete tx;
    }
};

class FilterMon {
private:
    VFIR_filter* dut;
    FilterScb* scb;
public:
    FilterMon(VFIR_filter* dut, FilterScb* scb): dut(dut), scb(scb) {}

    void monitorIn() {
        FilterInTx* tx = new FilterInTx();
        tx->Data_In = dut->Data_In;
        scb->writeIn(tx);
    }

    void monitorOut() {
        FilterOutTx* tx = new FilterOutTx();
        tx->Data_Out = dut->Data_Out;
        scb->writeOut(tx);
    }
};


// Generate sine wave input instead
FilterInTx* randomFilterInTx() {
    FilterInTx* tx = new FilterInTx();

    // Calculate sample from sine wave
    double val = sin(2.0 * PI * sine_freq * sine_phase);
    int amplitude = 127; // Half scale for 8-bit unsigned

    // Convert to 8-bit unsigned (centered at 128)
    tx->Data_In = static_cast<uint8_t>(val * amplitude + 128);

    sine_phase += sample_rate;
    return tx;
}

void dut_reset(VFIR_filter* dut, vluint64_t sim_time) {
    dut->reset = 0;
    if (sim_time >= 2 && sim_time < 6) {
        dut->reset = 1;
    }
}

int main(int argc, char** argv, char** env) {
    srand(time(NULL));
    Verilated::commandArgs(argc, argv);

    VFIR_filter* dut = new VFIR_filter;
    Verilated::traceEverOn(true);
    VerilatedVcdC* m_trace = new VerilatedVcdC;
    dut->trace(m_trace, 5);
    m_trace->open("waveform.vcd");

    FilterDrv* drv = new FilterDrv(dut);
    FilterScb* scb = new FilterScb();
    FilterMon* mon = new FilterMon(dut, scb);

    dut->clock = 0;

    sine_freq = 0.01 + ((rand() % 24) / 100.0); // Random freq between 0.01 and 0.25


    while (sim_time < MAX_SIM_TIME) {
        dut_reset(dut, sim_time);

        dut->clock ^= 1;
        dut->eval();

        if (dut->clock == 1) {
            if (sim_time >= VERIF_START_TIME) {
                FilterInTx* tx = randomFilterInTx();
                drv->drive(tx);
                mon->monitorIn();
                mon->monitorOut();
            }
        }

        m_trace->dump(sim_time);
        sim_time++;
    }

    m_trace->close();
    delete dut;
    delete drv;
    delete mon;
    delete scb;
    std::cout << "Generated sine frequency: " << sine_freq << " cycles/sample" << std::endl;

    return 0;
}
