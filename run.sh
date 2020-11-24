
make && ./yosys -p "read_verilog ../curtl/experiments/verilog/test.v; write_cxxrtl test.cpp; write_ilang test.ilang; small_nets"
