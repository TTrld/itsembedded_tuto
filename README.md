# FIR Filter Simulation with Verilator

This repository contains the final result of following the [itsembedded Verilator tutorial](https://www.itsembedded.com/dhd/verilator_1/) and applying the process to a FIR (Finite Impulse Response) filter module.

## Overview

The project demonstrates how to simulate a simple FIR filter design using Verilator, a high-performance SystemVerilog simulator. The simulation includes waveform generation and a basic testbench to verify functionality. I viewed this during my internship at SAL.

Dependencies: you need to have verilator, gtkwave, gcc, make installed on your system. 
Written and tested primarily under Arch Linux, known to be working on Ubuntu 20.04 and 18.04.

To simulate the testbench, simply run `make` in the root of the repo.
To draw waveforms, run `make waves`.

### Installing GTKWave from Source

If GTKWave is not available via your package manager, you can build it from source:

```bash
git clone https://github.com/gtkwave/gtkwave.git
cd gtkwave
./configure
make
sudo make install