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

/*
    The goal of cuRTL is do everything with 32-bit integers.
    If there are nets that are larger than 32-bit, or if there are
    operations input or outputs that are larger 32-bits, these need to be broken up
    into smaller parts.

*/


void list_cell_connections(RTLIL::Module *module)
{
    SigMap sigmap(module);

    for(auto cell: module->cells()){
        log("cell: %s\n", log_id(cell));
        for(auto &conn : cell->connections()){
            // conn = const dict<RTLIL::IdString, RTLIL::SigSpec> &RTLIL::Cell::connections() const

            SigSpec sigspec = conn.second;

            log("    %s -> %d\n", log_id(conn.first), sigspec.size());

        }
    }
}

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
                    log("Listing cell connections of %s\n", log_id(module));
                    list_cell_connections(module);
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
