
make && ./yosys -p "read_verilog ../curtl/experiments/verilog/test.v; write_cxxrtl test.cpp; small_nets"
