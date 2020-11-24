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

#if 0
        SigBit a = module->addWire(NEW_ID);
        SigBit b = module->addWire(NEW_ID);
        module->connect(a,b);
#endif
#if 1
        if(!cell->type.in(ID($reduce_or))){
            return;
        }
    
        RTLIL::SigSpec inputA = cell->getPort(ID::A);
    
        if (inputA.size() > max_width){
            RTLIL::Cell * cell_lsb   = module->addCell(NEW_ID, ID($reduce_or));
            RTLIL::Wire * lsb_y = module->addWire(NEW_ID, 1);
            RTLIL::SigSpec lsb_y_s = RTLIL::SigSpec(lsb_y);

            RTLIL::Cell * cell_msb   = module->addCell(NEW_ID, ID($reduce_or));
            RTLIL::Wire * msb_y = module->addWire(NEW_ID, 1);
            RTLIL::SigSpec msb_y_s = RTLIL::SigSpec(msb_y);
    
            cell_lsb->setPort(ID::A, inputA.extract(0, max_width));
            cell_lsb->setPort(ID::Y, lsb_y_s);
            cell_lsb->parameters[ID::A_SIGNED] = RTLIL::Const(0);
            cell_lsb->parameters[ID::A_WIDTH] = RTLIL::Const(max_width);
            cell_lsb->parameters[ID::Y_WIDTH] = RTLIL::Const(1);

            cell_msb->setPort(ID::A, inputA.extract_end(max_width));
            cell_msb->setPort(ID::Y, msb_y_s);
            cell_msb->parameters[ID::A_SIGNED] = RTLIL::Const(0);
            cell_msb->parameters[ID::A_WIDTH] = RTLIL::Const(inputA.size()-max_width);
            cell_msb->parameters[ID::Y_WIDTH] = RTLIL::Const(1);

            RTLIL::SigSpec finalA = RTLIL::SigSpec(lsb_y_s);
            finalA.append(msb_y_s);

            cell->setPort(ID::A, finalA);
            cell->parameters[ID::A_WIDTH] = RTLIL::Const(2);

            log_cell(cell_lsb);
            log_cell(cell_msb);
            log_cell(cell);
        }
#endif
    }
    
    
    void list_cell_connections(RTLIL::Module *module)
    {
        std::vector<RTLIL::Cell*> cells;

        for(RTLIL::Cell *cell: module->cells()){
            log("cell: %s (%s)\n", log_id(cell), log_id(cell->type));
            bool too_large = false;
            for(auto &conn : cell->connections()){
                // conn = const dict<RTLIL::IdString, RTLIL::SigSpec> &RTLIL::Cell::connections() const
    
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
#if 0
		CellTypes ct(design);
		for (auto module : design->selected_modules())
		{
			log("Optimizing module %s.\n", log_id(module));

			if (undriven) {
				did_something = false;
				replace_undriven(module, ct);
				if (did_something)
					design->scratchpad_set_bool("opt.did_something", true);
			}

			do {
				do {
					did_something = false;
					replace_const_cells(design, module, false /* consume_x */, mux_undef, mux_bool, do_fine, keepdc, noclkinv);
					if (did_something)
						design->scratchpad_set_bool("opt.did_something", true);
				} while (did_something);
				if (!keepdc)
					replace_const_cells(design, module, true /* consume_x */, mux_undef, mux_bool, do_fine, keepdc, noclkinv);
				if (did_something)
					design->scratchpad_set_bool("opt.did_something", true);
			} while (did_something);

			log_suppressed();
		}
#endif

		log_pop();
	}
} SmallNets;

PRIVATE_NAMESPACE_END
