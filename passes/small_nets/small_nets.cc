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

    void single_input_to_1bit(RTLIL::Module *module, RTLIL::Cell *cell, int max_width)
    {
        RTLIL::SigSpec inputA  = cell->getPort(ID::A);
        RTLIL::SigSpec outputY = cell->getPort(ID::Y);

        bool a_oversized = inputA.size()  > max_width;
        bool y_oversized = outputY.size() > max_width;

        if (a_oversized){

            IdString cell_type_lsb;
            IdString cell_type_msb;
            IdString cell_type_merge;

            if(cell->type.in(ID($reduce_or), ID($reduce_and), ID($reduce_xor), ID($reduce_xnor), ID($reduce_bool) )){
                cell_type_lsb   = cell->type;
                cell_type_msb   = cell->type;
                cell_type_merge = cell->type;
            }
            else if(cell->type.in(ID($logic_not) )){
                cell_type_lsb   = cell->type;
                cell_type_msb   = cell->type;
                cell_type_merge = ID($reduce_and);
            }

            RTLIL::Cell * cell_lsb  = module->addCell(NEW_ID, cell_type_lsb);
            RTLIL::Wire * y_lsb     = module->addWire(NEW_ID, 1);

            RTLIL::Cell * cell_msb  = module->addCell(NEW_ID, cell_type_msb);
            RTLIL::Wire * y_msb     = module->addWire(NEW_ID, 1);
    
            cell_lsb->setPort(ID::A, inputA.extract(0, max_width));
            cell_lsb->setPort(ID::Y, y_lsb);
            cell_lsb->parameters[ID::A_SIGNED] = RTLIL::Const(0);
            cell_lsb->parameters[ID::A_WIDTH] = RTLIL::Const(max_width);
            cell_lsb->parameters[ID::Y_WIDTH] = RTLIL::Const(1);

            cell_msb->setPort(ID::A, inputA.extract_end(max_width));
            cell_msb->setPort(ID::Y, y_msb);
            cell_msb->parameters[ID::A_SIGNED] = RTLIL::Const(0);
            cell_msb->parameters[ID::A_WIDTH] = RTLIL::Const(inputA.size()-max_width);
            cell_msb->parameters[ID::Y_WIDTH] = RTLIL::Const(1);

            RTLIL::SigSpec finalA = RTLIL::SigSpec(y_lsb);
            finalA.append(y_msb);

            cell->type = cell_type_merge;
            cell->setPort(ID::A, finalA);
            cell->parameters[ID::A_WIDTH] = RTLIL::Const(2);

            log_cell(cell_lsb);
            log_cell(cell_msb);
            log_cell(cell);
        }

        if (y_oversized){
            RTLIL::SigSpec y_lsb;
            y_lsb = outputY.extract(0, max_width);
            cell->parameters[ID::Y_WIDTH] = y_lsb.size();
            cell->setPort(ID::Y, y_lsb);

            outputY.remove(0, max_width);
            module->connect(outputY, Const(0, GetSize(outputY)));
        }
    }

    void bitwise_ops(RTLIL::Module *module, RTLIL::Cell *cell, int max_width)
    {
        RTLIL::SigSpec inputA  = cell->getPort(ID::A);
        RTLIL::SigSpec inputB  = cell->getPort(ID::B);
        RTLIL::SigSpec outputY = cell->getPort(ID::Y);

        bool a_oversized = inputA.size()  > max_width;
        bool b_oversized = inputB.size()  > max_width;
        bool y_oversized = outputY.size() > max_width;

        if (a_oversized || b_oversized || y_oversized){

            IdString cell_type_lsb = cell->type;
            IdString cell_type_msb = cell->type;

            RTLIL::Cell * cell_lsb  = module->addCell(NEW_ID, cell_type_lsb);

            RTLIL::SigSpec a_lsb, a_msb;

            if (a_oversized){
                a_lsb = inputA.extract(0,max_width);
                a_msb = inputA.extract_end(max_width);
            }
            else{
                a_lsb = inputA;

                if (cell->parameters[ID::A_SIGNED] == 1){
                    a_msb = inputA.extract(inputA.size()-1);
                }
                else{
                    a_msb = RTLIL::Const(0);
                }
            }

            RTLIL::SigSpec b_lsb, b_msb;

            if (b_oversized){
                b_lsb = inputB.extract(0,max_width);
                b_msb = inputB.extract_end(max_width);
            }
            else{
                b_lsb = inputB;

                if (cell->parameters[ID::B_SIGNED] == 1){
                    b_msb = inputB.extract(inputB.size()-1);
                }
                else{
                    b_msb = RTLIL::Const(0);
                }
            }

            cell_lsb->setPort(ID::A, a_lsb);
            cell_lsb->parameters[ID::A_SIGNED] = cell->parameters[ID::A_SIGNED];
            cell_lsb->parameters[ID::A_WIDTH] = a_lsb.size();

            cell_lsb->setPort(ID::B, b_lsb);
            cell_lsb->parameters[ID::B_SIGNED] = cell->parameters[ID::B_SIGNED];
            cell_lsb->parameters[ID::B_WIDTH] = b_lsb.size();

            if (y_oversized){
                RTLIL::SigSpec y_lsb, y_msb;
                y_lsb = outputY.extract(0, max_width);
                y_msb = outputY.extract_end(max_width);

                cell_lsb->setPort(ID::Y, y_lsb);
                cell_lsb->parameters[ID::Y_WIDTH] = y_lsb.size();

                RTLIL::Cell * cell_msb  = module->addCell(NEW_ID, cell_type_msb);

                cell_msb->setPort(ID::A, a_msb);
                cell_msb->parameters[ID::A_SIGNED] = cell->parameters[ID::A_SIGNED];
                cell_msb->parameters[ID::A_WIDTH] = a_msb.size();

                cell_msb->setPort(ID::B, b_msb);
                cell_msb->parameters[ID::B_SIGNED] = cell->parameters[ID::B_SIGNED];
                cell_msb->parameters[ID::B_WIDTH] = b_msb.size();

                cell_msb->setPort(ID::Y, y_msb);
                cell_msb->parameters[ID::Y_WIDTH] = y_msb.size();
            }
            else{
                RTLIL::SigSpec y_lsb;
                y_lsb = outputY;

                cell_lsb->setPort(ID::Y, y_lsb);
                cell_lsb->parameters[ID::Y_WIDTH] = y_lsb.size();
            }

            module->remove(cell);
        }
    }


    void logic_and_or(RTLIL::Module *module, RTLIL::Cell *cell, int max_width)
    {
        RTLIL::SigSpec inputA  = cell->getPort(ID::A);
        RTLIL::SigSpec inputB  = cell->getPort(ID::B);
        RTLIL::SigSpec outputY = cell->getPort(ID::Y);

        bool a_oversized = inputA.size()  > max_width;
        bool b_oversized = inputB.size()  > max_width;
        bool y_oversized = outputY.size() > max_width;

        // In the first pass, the oversized input to $logic_XXX gets
        // replaced by a single bit $reduce_bool output. The input
        // to $reduce_bool will still be oversized, but that will be
        // split up when small_nets is called again.

        if (a_oversized){
            RTLIL::Cell * cell_bool  = module->addCell(NEW_ID, ID($reduce_bool));
            RTLIL::Wire * y_bool = module->addWire(NEW_ID, 1);

            cell_bool->parameters[ID::A_SIGNED] = cell->parameters[ID::A_SIGNED];
            cell_bool->parameters[ID::A_WIDTH]  = cell->parameters[ID::A_WIDTH];
            cell_bool->setPort(ID::A, inputA);

            cell_bool->parameters[ID::Y_WIDTH]  = 1;
            cell_bool->setPort(ID::Y, y_bool);

            cell->parameters[ID::A_WIDTH]  = 1;
            cell->setPort(ID::A, y_bool);
        }

        if (b_oversized){
            RTLIL::Cell * cell_bool  = module->addCell(NEW_ID, ID($reduce_bool));
            RTLIL::Wire * y_bool = module->addWire(NEW_ID, 1);

            cell_bool->parameters[ID::A_SIGNED] = cell->parameters[ID::B_SIGNED];
            cell_bool->parameters[ID::A_WIDTH]  = cell->parameters[ID::B_WIDTH];
            cell_bool->setPort(ID::A, inputB);

            cell_bool->parameters[ID::Y_WIDTH]  = 1;
            cell_bool->setPort(ID::Y, y_bool);

            cell->parameters[ID::B_WIDTH]  = 1;
            cell->setPort(ID::B, y_bool);
        }

        if (y_oversized){
            RTLIL::SigSpec y_lsb;
            y_lsb = outputY.extract(0, max_width);
            cell->parameters[ID::Y_WIDTH] = y_lsb.size();
            cell->setPort(ID::Y, y_lsb);

            outputY.remove(0, max_width);
            module->connect(outputY, Const(0, GetSize(outputY)));
        }
    }

    void add_sub(RTLIL::Module *module, RTLIL::Cell *cell, int max_width)
    {
        RTLIL::SigSpec inputA  = cell->getPort(ID::A);
        RTLIL::SigSpec inputB  = cell->getPort(ID::B);
        RTLIL::SigSpec outputY = cell->getPort(ID::Y);

        bool a_oversized = inputA.size()  > max_width;
        bool b_oversized = inputB.size()  > max_width;

        // A and B oversized:
        //
        // y[10:0] = a[10:0] + b[10:0]
        // max_width = 8: 
        // { carry, y[6:0] } = a[6:0] + b[6:0]
        // { y[10:7], dummy } = { a[10:7], 1'b1 } + { b[10:7], carry }

        // A oversized, B is max_width
        //
        // y[10:0] = a[10:0] + b[7:0]
        // max_width = 8: 
        // { carry, y[6:0] } = a[6:0] + b[6:0]
        // { y[10:7], dummy } = { a[10:7], 1'b1 } + { b[10:7], carry }

        // A oversized, B > max_width
        //
        // y[10:0] = a[10:0] + b[6:0]
        // max_width = 8: 
        // { carry, y[6:0] } = a[6:0] + b[6:0]
        // { y[10:7], dummy } = { a[10:7], 1'b1 } + { b[10:7], carry }


        // Clip A and B to be equal or less than Y
        if (inputA.size() > outputY.size() || inputB.size() > outputY.size()){

            if (inputA.size() > outputY.size()){
                RTLIL::SigSpec a_reduced = inputA.extract(0,outputY.size());
                cell->parameters[ID::A_WIDTH] = outputY.size();
                cell->setPort(ID::A, a_reduced);
            }

            if (inputB.size() > outputY.size()){
                RTLIL::SigSpec b_reduced = inputB.extract(0,outputY.size());
                cell->parameters[ID::B_WIDTH] = outputY.size();
                cell->setPort(ID::B, b_reduced);
            }
        }
        else if (a_oversized || b_oversized){
            // Due to the previous case, we know for sure that Y is also oversized, but
            // never larger than A or B.

            RTLIL::SigSpec a_lsb, a_msb;

            if (a_oversized || (b_oversized && inputA.size() == max_width)){
                a_lsb = inputA.extract(0,max_width-1);
                a_msb = inputA.extract_end(max_width-1);
            }
            else{
                a_lsb = RTLIL::SigSpec(inputA);

                if (cell->parameters[ID::A_SIGNED] == 1){
                    a_lsb.extend_u0(max_width-1, 1);
                    a_msb = inputA.extract(inputA.size()-1);
                }
                else{
                    a_msb = RTLIL::Const(0);
                }
            }
            a_lsb.append(RTLIL::Const(0));

            RTLIL::SigSpec b_lsb, b_msb;

            if (b_oversized || (a_oversized && inputB.size() == max_width)){
                b_lsb = inputB.extract(0,max_width-1);
                b_msb = inputB.extract_end(max_width-1);
            }
            else{
                b_lsb = RTLIL::SigSpec(inputB);

                if (cell->parameters[ID::B_SIGNED] == 1){
                    b_lsb.extend_u0(max_width-1, 1);
                    b_msb = inputB.extract(inputB.size()-1);
                }
                else{
                    b_msb = RTLIL::Const(0);
                }
            }
            b_lsb.append(RTLIL::Const(0));

            // { carry, y_lsb} = a_lsb + b_lsb
            // { y_msb, dummy } = { a_msb, 1'b1 } + { b_msb, carry }

            RTLIL::Wire *carry = module->addWire(NEW_ID);
            RTLIL::SigSpec y_lsb = outputY.extract(0, max_width-1);

            RTLIL::SigSpec combo_out_lsb = RTLIL::SigSpec(y_lsb);
            combo_out_lsb.append(carry);

            IdString cell_type_lsb = cell->type;
            RTLIL::Cell * cell_lsb  = module->addCell(NEW_ID, cell_type_lsb);

            cell_lsb->setPort(ID::A, a_lsb);
            cell_lsb->parameters[ID::A_SIGNED] = cell->parameters[ID::A_SIGNED];
            cell_lsb->parameters[ID::A_WIDTH] = a_lsb.size();

            cell_lsb->setPort(ID::B, b_lsb);
            cell_lsb->parameters[ID::B_SIGNED] = cell->parameters[ID::B_SIGNED];
            cell_lsb->parameters[ID::B_WIDTH] = b_lsb.size();

            cell_lsb->setPort(ID::Y, combo_out_lsb);
            cell_lsb->parameters[ID::Y_WIDTH] = combo_out_lsb.size();

            RTLIL::Wire *dummy = module->addWire(NEW_ID);
            RTLIL::SigSpec y_msb = outputY.extract_end(max_width-1);

            RTLIL::SigSpec combo_out_msb = RTLIL::SigSpec(dummy);
            combo_out_msb.append(y_msb);

            RTLIL::SigSpec combo_a_msb = RTLIL::Const(1,1);
            combo_a_msb.append(a_msb);

            RTLIL::SigSpec combo_b_msb = carry;
            combo_b_msb.append(b_msb);

            IdString cell_type_msb = cell->type;
            RTLIL::Cell * cell_msb  = module->addCell(NEW_ID, cell_type_msb);

            cell_msb->setPort(ID::A, combo_a_msb);
            cell_msb->parameters[ID::A_SIGNED] = cell->parameters[ID::A_SIGNED];
            cell_msb->parameters[ID::A_WIDTH] = combo_a_msb.size();

            cell_msb->setPort(ID::B, combo_b_msb);
            cell_msb->parameters[ID::B_SIGNED] = cell->parameters[ID::B_SIGNED];
            cell_msb->parameters[ID::B_WIDTH] = combo_b_msb.size();

            cell_msb->setPort(ID::Y, combo_out_msb);
            cell_msb->parameters[ID::Y_WIDTH] = combo_out_msb.size();

            module->remove(cell);
        }
    }

    void split_or_reduce(RTLIL::Module *module, RTLIL::Cell *cell, int max_width)
    {
        if(cell->type.in(ID($reduce_or), ID($reduce_and), ID($reduce_xor), ID($reduce_xnor), ID($reduce_bool),
                         ID($logic_not) ))
        {
            single_input_to_1bit(module, cell, max_width);
        }
        else if (cell->type.in(ID($and),ID($or),ID($xor),ID($xnor)))
        {
            bitwise_ops(module, cell, max_width);
        }
        else if (cell->type.in(ID($logic_and),ID($logic_or)))
        {
            logic_and_or(module, cell, max_width);
        }
        else if (cell->type.in(ID($add)))
        {
            add_sub(module, cell, max_width);
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
