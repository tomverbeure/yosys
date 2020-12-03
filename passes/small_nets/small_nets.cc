/*
 *  yosys -- Yosys Open SYnthesis Suite
 *
 *  Copyright (C) 2012  Clifford Wolf <clifford@clifford.at>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "kernel/register.h"
#include "kernel/sigtools.h"
#include "kernel/celltypes.h"
#include "kernel/utils.h"
#include "kernel/log.h"
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct SmallNetsWorker
{

/*
    The goal of cuRTL is do everything with 32-bit integers.
    If there are nets that are larger than 32-bit, or if there are
    operations input or outputs that are larger 32-bits, these need to be broken up
    into smaller parts.

*/

    void split_or_reduce(RTLIL::Module *module, RTLIL::Cell *cell, int max_width)
    {
        if(cell->type.in(ID($reduce_or))){
            RTLIL::SigSpec inputA = cell->getPort(ID::A);
    
            // Split single port in one that conforms to max_width and one with all the rest.
            if (inputA.size() > max_width){
                RTLIL::Cell * cell_lsb  = module->addCell(NEW_ID, ID($reduce_or));
                RTLIL::Wire * lsb_y     = module->addWire(NEW_ID, 1);
    
                RTLIL::Cell * cell_msb  = module->addCell(NEW_ID, ID($reduce_or));
                RTLIL::Wire * msb_y     = module->addWire(NEW_ID, 1);
        
                cell_lsb->setPort(ID::A, inputA.extract(0, max_width));
                cell_lsb->setPort(ID::Y, lsb_y);
                cell_lsb->parameters[ID::A_SIGNED] = RTLIL::Const(0);
                cell_lsb->parameters[ID::A_WIDTH] = RTLIL::Const(max_width);
                cell_lsb->parameters[ID::Y_WIDTH] = RTLIL::Const(1);
    
                cell_msb->setPort(ID::A, inputA.extract_end(max_width));
                cell_msb->setPort(ID::Y, msb_y);
                cell_msb->parameters[ID::A_SIGNED] = RTLIL::Const(0);
                cell_msb->parameters[ID::A_WIDTH] = RTLIL::Const(inputA.size()-max_width);
                cell_msb->parameters[ID::Y_WIDTH] = RTLIL::Const(1);
    
                RTLIL::SigSpec finalA = RTLIL::SigSpec(lsb_y);
                finalA.append(msb_y);
    
                cell->setPort(ID::A, finalA);
                cell->parameters[ID::A_WIDTH] = RTLIL::Const(2);

                log_cell(cell_lsb);
                log_cell(cell_msb);
                log_cell(cell);
            }
        }
    }
    
    
    void list_cell_connections(RTLIL::Module *module)
    {
#if 1
        std::vector<RTLIL::Cell*> cells;

        for(RTLIL::Cell *cell: module->cells()){
            log("cell: %s (%s)\n", log_id(cell), log_id(cell->type));
            bool too_large = false;
            for(auto &conn : cell->connections()){
                SigSpec sigspec = conn.second;
    
                log("    %s -> %d\n", log_id(conn.first), sigspec.size());
    
                if (sigspec.size() > 32){
                    too_large = true;
                }
            }
            if (too_large){
                cells.push_back(cell);
            }
        }

        for(auto cell : cells){
            split_or_reduce(module, cell, 32);
        }
#endif

#if 0
        // Experiment: create a new port driven by a reduce_xor.
        RTLIL::Cell * cell_xor = module->addCell(NEW_ID, ID($reduce_xor));
        cell_xor->setPort(ID::A, RTLIL::Const(1, 2));
        cell_xor->parameters[ID::A_SIGNED] = RTLIL::Const(1);
        cell_xor->parameters[ID::A_WIDTH] = RTLIL::Const(2);
        cell_xor->parameters[ID::Y_WIDTH] = RTLIL::Const(1);

        RTLIL::Wire *test_out = module->addWire(ID(test_out));
        test_out->port_output = true;
        SigSpec test_out_s = SigSpec(test_out);

        cell_xor->setPort(ID::Y, test_out_s);

        module->fixup_ports();
#endif
    }
};

struct SmallNets : public Pass {
	SmallNets() : Pass("small_nets", "perform const folding and simple expression rewriting") { }
	void help() override
	{
		//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
		log("\n");
		log("    small_nets [options] [selection]\n");
		log("\n");
		log("\n");
	}

	void execute(std::vector<std::string> args, RTLIL::Design *design) override
	{
		bool keepdc = false;

		log_header(design, "Executing OPT_EXPR pass (perform const folding).\n");
		log_push();

		size_t argidx;
		for (argidx = 1; argidx < args.size(); argidx++) {
			if (args[argidx] == "-keepdc") {
				keepdc = true;
				continue;
			}
			break;
		}
		extra_args(args, argidx, design);

                for(auto module: design->selected_modules()){
		    if (module->has_processes_warn())
		        continue;

                    SmallNetsWorker worker;

                    log("Listing cell connections of %s\n", log_id(module));
                    worker.list_cell_connections(module);
                }

		log_pop();
	}
} SmallNets;

PRIVATE_NAMESPACE_END
