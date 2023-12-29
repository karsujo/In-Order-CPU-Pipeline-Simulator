/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apex_cpu.h"
#include "apex_macros.h"

/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}

static void
print_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
    case OPCODE_ADD:
    case OPCODE_SUB:
    case OPCODE_MUL:
    case OPCODE_DIV:
    case OPCODE_AND:
    case OPCODE_OR:
    case OPCODE_XOR:
    {
        printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
               stage->rs2);
        break;
    }

    case OPCODE_ADDL:
    {
        printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
               stage->imm);
        break;
    }

    case OPCODE_SUBL:
    {
        printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
               stage->imm);
        break;
    }

    case OPCODE_MOVC:
    {
        printf("%s,R%d,#%d ", stage->opcode_str, stage->rd, stage->imm);
        break;
    }

    case OPCODE_LOAD:
    {
        printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
               stage->imm);
        break;
    }

    case OPCODE_STORE:
    {
        printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
               stage->imm);
        break;
    }

    case OPCODE_LOADP:
    {
        printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
               stage->imm);
        break;
    }

    case OPCODE_STOREP:
    {
        printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
               stage->imm);
        break;
    }

    case OPCODE_BZ:
    case OPCODE_BNZ:
    case OPCODE_BP:
    case OPCODE_BNP:
    case OPCODE_BN:
    case OPCODE_BNN:
    {
        printf("%s,#%d ", stage->opcode_str, stage->imm);
        break;
    }
    case OPCODE_CMP:
    {
        printf("%s,R%d,R%d", stage->opcode_str, stage->rs1, stage->rs2);
        break;
    }
    case OPCODE_CML:
    {
        printf("%s,R%d,#%d", stage->opcode_str, stage->rs1, stage->imm);
        break;
    }
    case OPCODE_HALT:
    {
        printf("%s", stage->opcode_str);
        break;
    }
    case OPCODE_JALR:
    {
        printf("%s,R%d,R%d,#%d", stage->opcode_str, stage->rd, stage->rs1, stage->imm);
        break;
    }
    case OPCODE_JUMP:
    {
        printf("%s,R%d,#%d", stage->opcode_str, stage->rs1, stage->imm);
        break;
    }
    case OPCODE_NOP:
    {
        printf("%s", stage->opcode_str);
        break;
    }
    }
}

/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const char *name, const CPU_Stage *stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
static void
print_reg_file(const APEX_CPU *cpu)
{
    int i;

    printf("----------\n%s\n----------\n", "Registers:");

    for (int i = 0; i < REG_FILE_SIZE / 2; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");

    for (i = (REG_FILE_SIZE / 2); i < REG_FILE_SIZE; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");
}

static void
print_data_memory(const APEX_CPU *cpu)
{
    printf("----------\n%s\n----------\n", "Data Memory:");

    for (int i = 0; i < DATA_MEMORY_SIZE; ++i)
    {
        if(cpu->data_memory[i]!=0){
        printf("%-3d[%-3d] ", i, cpu->data_memory[i]);
        }
    }

    printf("\n");
}

static void
print_flag_values(const APEX_CPU *cpu)
{
    printf("----------\n%s\n----------\n", "Flags:");

    printf("P->[%d], Z->[%d], N->[%d]", cpu->positive_flag, cpu->zero_flag, cpu->negative_flag);

    printf("\n");
}

void FORWARDED_DECODER_MUX_RS1(APEX_CPU *cpu)
{
        if(cpu->memory.has_insn)
        {   
            //don't get rd from EX if instruction is LOADP/LAOD -- LOADP only resolves rd in MEM stage
            if((cpu->memory.opcode!=OPCODE_LOADP && cpu->memory.opcode!=OPCODE_LOAD) && cpu->decode.rs1 == cpu->memory.rd)
            {
                cpu->decode.rs1_value = cpu->execute.result_buffer; 
                cpu->regs_state[cpu->decode.rs1] = 0; 
                return;
            }
            else if(cpu->memory.opcode==OPCODE_STOREP && cpu->decode.rs1 == cpu->memory.rs2)
            {
                cpu->decode.rs1_value = cpu->memory.rs2_value; 
                cpu->regs_state[cpu->decode.rs1] = 0; 
                return;
            }
            else if(cpu->memory.opcode==OPCODE_LOADP && cpu->decode.rs1 == cpu->memory.rs1)
            {
                cpu->decode.rs1_value = cpu->memory.rs1_value; 
                cpu->regs_state[cpu->decode.rs1] = 0; 
                return;
            }
        }
        if(cpu->writeback.has_insn)
         {
            if(cpu->decode.rs1 == cpu->writeback.rd)
            {
                cpu->decode.rs1_value = cpu->writeback.result_buffer; 
                cpu->regs_state[cpu->decode.rs1] = 0; 
                return;
            }
            else if(cpu->writeback.opcode==OPCODE_STOREP && cpu->decode.rs1 == cpu->writeback.rs2)
            {
                cpu->decode.rs1_value = cpu->writeback.rs2_value; 
                cpu->regs_state[cpu->decode.rs1] = 0; 
                return;
            }
            else if(cpu->writeback.opcode==OPCODE_LOADP && cpu->decode.rs1 == cpu->writeback.rs1)
            {
                cpu->decode.rs1_value = cpu->writeback.rs1_value; 
                cpu->regs_state[cpu->decode.rs1] = 0; 
                return; 
            }
         }

            cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
}

void FORWARDED_DECODER_MUX_RS2(APEX_CPU *cpu)
{
       if(cpu->memory.has_insn)
        {
            //don't get rd from EX if instruction is LOADP/LAOD -- LOADP only resolves rd in MEM stage
            if((cpu->memory.opcode!=OPCODE_LOADP && cpu->memory.opcode!=OPCODE_LOAD) && cpu->decode.rs2 == cpu->memory.rd)            
            {
                cpu->decode.rs2_value = cpu->memory.result_buffer; 
                cpu->regs_state[cpu->decode.rs2] = 0; 
                return;
            }
            else if(cpu->memory.opcode==OPCODE_STOREP && cpu->decode.rs2 == cpu->memory.rs2)
            {
                cpu->decode.rs2_value = cpu->memory.rs2_value; 
                cpu->regs_state[cpu->decode.rs2] = 0; 
                return;
            }
            else if(cpu->memory.opcode==OPCODE_LOADP && cpu->decode.rs2 == cpu->memory.rs1)
            {
                cpu->decode.rs2_value = cpu->memory.rs1_value; 
                cpu->regs_state[cpu->decode.rs2] = 0; 
                return;
            }
        }
        if(cpu->writeback.has_insn)
        {
            if(cpu->decode.rs2 == cpu->writeback.rd)
            {
                cpu->decode.rs2_value = cpu->writeback.result_buffer; 
                cpu->regs_state[cpu->decode.rs2] = 0; 
                return;
            }
            else if(cpu->writeback.opcode==OPCODE_STOREP && cpu->decode.rs2 == cpu->writeback.rs2)
            {
                cpu->decode.rs2_value = cpu->writeback.rs2_value; 
                cpu->regs_state[cpu->decode.rs2] = 0; 
                return;
            }
            else if(cpu->writeback.opcode==OPCODE_LOADP && cpu->decode.rs2 == cpu->writeback.rs1)
            {
                cpu->decode.rs2_value = cpu->writeback.rs1_value; 
                cpu->regs_state[cpu->decode.rs2] = 0; 
                return;
            }
        }

        cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

}


/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_fetch(APEX_CPU *cpu)
{
    APEX_Instruction *current_ins;

    if ((cpu->fetch.has_insn))
    {
        if (cpu->stall_pipeline == 1)
        {
            cpu->fetch_before_stall = 0; // stall fetches from the next cycle.
        }
        /* This fetches new branch target instruction from next cycle */
        if (cpu->fetch_from_next_cycle == TRUE)
        {
            cpu->fetch_from_next_cycle = FALSE;

            /* Skip this cycle*/
            return;
        }

        /* Store current PC in fetch latch */
        cpu->fetch.pc = cpu->pc;

        /* Index into code memory using this pc and copy all instruction fields
         * into fetch latch  */
        current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
        strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
        cpu->fetch.opcode = current_ins->opcode;
        cpu->fetch.rd = current_ins->rd;
        cpu->fetch.rs1 = current_ins->rs1;
        cpu->fetch.rs2 = current_ins->rs2;
        cpu->fetch.imm = current_ins->imm;

        if (cpu->stall_pipeline == 0)
        {
            /* Update PC for next instruction */
            cpu->pc += 4;

            /* Copy data from fetch latch to decode latch*/
            cpu->decode = cpu->fetch;
        }

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Fetch", &cpu->fetch);
        }

        /* Stop fetching new instructions if HALT is fetched */
        if (cpu->fetch.opcode == OPCODE_HALT && cpu->stall_pipeline == 0)
        {
            cpu->fetch.has_insn = FALSE;
        }
    }
}

/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode(APEX_CPU *cpu)
{
    if (cpu->decode.has_insn)
    {
        /* Read operands from register file based on the instruction type */

        /* Copy data from decode latch to execute latch*/
        switch (cpu->decode.opcode)
        {
        case OPCODE_ADD:
        {

            FORWARDED_DECODER_MUX_RS1(cpu);
            FORWARDED_DECODER_MUX_RS2(cpu); 
            if (cpu->regs_state[cpu->decode.rs1] == 0 && cpu->regs_state[cpu->decode.rs2] == 0) // dest regs have been updated
            {
                cpu->regs_state[cpu->decode.rd] = 1;
                cpu->execute = cpu->decode;
                cpu->decode.has_insn = FALSE;
                cpu->stall_pipeline = 0;
            }
            else
            {
                cpu->stall_pipeline = 1;
            }
            break;
        }

        case OPCODE_ADDL:
        {
            FORWARDED_DECODER_MUX_RS1(cpu);
            if (cpu->regs_state[cpu->decode.rs1] == 0) // dest regs have been updated
            {
                cpu->regs_state[cpu->decode.rd] = 1;
                cpu->execute = cpu->decode;
                cpu->decode.has_insn = FALSE;
                cpu->stall_pipeline = 0;
            }
            else
            {
                cpu->stall_pipeline = 1;
            }
            break;
        }

        case OPCODE_SUB:
        {

            FORWARDED_DECODER_MUX_RS1(cpu);
            FORWARDED_DECODER_MUX_RS2(cpu); 
            if (cpu->regs_state[cpu->decode.rs1] == 0 && cpu->regs_state[cpu->decode.rs2] == 0) // dest regs have been updated
            {
                cpu->regs_state[cpu->decode.rd] = 1;
                cpu->execute = cpu->decode;
                cpu->decode.has_insn = FALSE;
                cpu->stall_pipeline = 0;
            }
            else
            {
                cpu->stall_pipeline = 1;
            }
            break;
        }

        case OPCODE_SUBL:
        {

            FORWARDED_DECODER_MUX_RS1(cpu);
            if (cpu->regs_state[cpu->decode.rs1] == 0) // dest regs have been updated
            {


                cpu->regs_state[cpu->decode.rd] = 1;
                cpu->execute = cpu->decode;
                cpu->decode.has_insn = FALSE;
                cpu->stall_pipeline = 0;
            }
            else
            {
                cpu->stall_pipeline = 1;
            }
            break;
        }

        case OPCODE_MUL:
        {
            FORWARDED_DECODER_MUX_RS1(cpu);
            FORWARDED_DECODER_MUX_RS2(cpu); 
            if (cpu->regs_state[cpu->decode.rs1] == 0 && cpu->regs_state[cpu->decode.rs2] == 0) // dest regs have been updated
            {
                cpu->regs_state[cpu->decode.rd] = 1;
                cpu->execute = cpu->decode;
                cpu->decode.has_insn = FALSE;
                cpu->stall_pipeline = 0;
            }
            else
            {
                cpu->stall_pipeline = 1;
            }
            break;
        }
        case OPCODE_XOR:
        {
            FORWARDED_DECODER_MUX_RS1(cpu);
            FORWARDED_DECODER_MUX_RS2(cpu);

            if (cpu->regs_state[cpu->decode.rs1] == 0 && cpu->regs_state[cpu->decode.rs2] == 0) // dest regs have been updated
            {
                cpu->regs_state[cpu->decode.rd] = 1;
                cpu->execute = cpu->decode;
                cpu->decode.has_insn = FALSE;
                cpu->stall_pipeline = 0;
            }
            else
            {
                cpu->stall_pipeline = 1;
            }
            break;
        }
        case OPCODE_OR:
        {
            FORWARDED_DECODER_MUX_RS1(cpu);
            FORWARDED_DECODER_MUX_RS2(cpu);
            if (cpu->regs_state[cpu->decode.rs1] == 0 && cpu->regs_state[cpu->decode.rs2] == 0) // dest regs have been updated
            {
                cpu->regs_state[cpu->decode.rd] = 1;
                cpu->execute = cpu->decode;
                cpu->decode.has_insn = FALSE;
                cpu->stall_pipeline = 0;
            }
            else
            {

                cpu->stall_pipeline = 1;
            }
            break;
        }
        case OPCODE_AND:
        {
            FORWARDED_DECODER_MUX_RS1(cpu);
            FORWARDED_DECODER_MUX_RS2(cpu);
            if (cpu->regs_state[cpu->decode.rs1] == 0 && cpu->regs_state[cpu->decode.rs2] == 0) // dest regs have been updated
            {
                cpu->regs_state[cpu->decode.rd] = 1;
                cpu->execute = cpu->decode;
                cpu->decode.has_insn = FALSE;
                cpu->stall_pipeline = 0;
            }
            else
            {
                cpu->stall_pipeline = 1;
            }
            break;
        }
        case OPCODE_MOVC:
        {

            cpu->regs_state[cpu->decode.rd] = 1;

            cpu->execute = cpu->decode;
            cpu->decode.has_insn = FALSE;

            break;
        }
        case OPCODE_CMP:
        {

            FORWARDED_DECODER_MUX_RS1(cpu);
            FORWARDED_DECODER_MUX_RS2(cpu);

            if (cpu->regs_state[cpu->decode.rs1] == 0 && cpu->regs_state[cpu->decode.rs2] == 0) // dest regs have been updated
            {
                cpu->execute = cpu->decode;
                // cpu->execute.has_insn = TRUE;
                cpu->decode.has_insn = FALSE;
                cpu->stall_pipeline = 0;
            }
            else
            {
                // if(cpu->stall_pipeline==0) // fetch and keep next instr only while starting the stall procedure
                // {
                //     cpu->fetch_before_stall = 1;
                // }
                cpu->stall_pipeline = 1;
            }
            break;
        }
        case OPCODE_CML:
        {
            FORWARDED_DECODER_MUX_RS1(cpu);

            if (cpu->regs_state[cpu->decode.rs1] == 0) // dest regs have been updated
            {
                cpu->execute = cpu->decode;
                // cpu->execute.has_insn = TRUE;
                cpu->decode.has_insn = FALSE;
                cpu->stall_pipeline = 0;
            }
            else
            {
                // if(cpu->stall_pipeline==0) // fetch and keep next instr only while starting the stall procedure
                // {
                //     cpu->fetch_before_stall = 1;
                // }
                cpu->stall_pipeline = 1;
            }
            break;
        }
        case OPCODE_JALR:
        {
            FORWARDED_DECODER_MUX_RS1(cpu);
            if (cpu->regs_state[cpu->decode.rs1] == 0) // dest regs have been updated
            {
                cpu->regs_state[cpu->decode.rd] = 1;
                cpu->execute = cpu->decode;
                cpu->decode.has_insn = FALSE;
                cpu->stall_pipeline = 0;
            }
            else
            {
                cpu->stall_pipeline = 1;
            }
            break;
        }

        case OPCODE_JUMP:
        {
            FORWARDED_DECODER_MUX_RS1(cpu);
            if (cpu->regs_state[cpu->decode.rs1] == 0) // dest regs have been updated
            {
                cpu->execute = cpu->decode;
                cpu->decode.has_insn = FALSE;
                cpu->stall_pipeline = 0;
            }
            else
            {
                cpu->stall_pipeline = 1;
            }
            break;
        }

        case OPCODE_BNZ:
        {
            cpu->execute = cpu->decode;
            // cpu->execute.has_insn = TRUE;
            cpu->decode.has_insn = FALSE;
            break;
        }
        case OPCODE_BNP:
        {
            cpu->execute = cpu->decode;
            // cpu->execute.has_insn = TRUE;
            cpu->decode.has_insn = FALSE;
            break;
        }
        case OPCODE_BNN:
        {
            cpu->execute = cpu->decode;
            // cpu->execute.has_insn = TRUE;
            cpu->decode.has_insn = FALSE;
            break;
        }
        case OPCODE_BP:
        {
            cpu->execute = cpu->decode;
            // cpu->execute.has_insn = TRUE;
            cpu->decode.has_insn = FALSE;
            break;
        }
        case OPCODE_BN:
        {
            cpu->execute = cpu->decode;
            // cpu->execute.has_insn = TRUE;
            cpu->decode.has_insn = FALSE;
            break;
        }

        case OPCODE_BZ:
        {
            cpu->execute = cpu->decode;
            // cpu->execute.has_insn = TRUE;
            cpu->decode.has_insn = FALSE;
            break;
        }
        case OPCODE_HALT:
        {
            cpu->execute = cpu->decode;
            // cpu->execute.has_insn = TRUE;
            cpu->decode.has_insn = FALSE;
            break;
        }
        case OPCODE_LOAD:
        {

            FORWARDED_DECODER_MUX_RS1(cpu);
            if (cpu->regs_state[cpu->decode.rs1] == 0)
            {
                cpu->regs_state[cpu->decode.rd] = 1;
                cpu->execute = cpu->decode;
                // cpu->execute.has_insn = TRUE;
                cpu->decode.has_insn = FALSE;
                cpu->stall_pipeline = 0;
            }
            else
            {
                cpu->stall_pipeline = 1;
            }
            break;
        }
        case OPCODE_STORE:
        {
            FORWARDED_DECODER_MUX_RS1(cpu);
            FORWARDED_DECODER_MUX_RS2(cpu);
            if (cpu->regs_state[cpu->decode.rs1] == 0 && cpu->regs_state[cpu->decode.rs2] == 0) // dest regs have been updated
            {

                cpu->execute = cpu->decode;
                cpu->decode.has_insn = FALSE;
                cpu->stall_pipeline = 0;

                break;
            }
            else
            {
                cpu->stall_pipeline = 1;
            }
            break;
        }
        case OPCODE_LOADP:
        {
            FORWARDED_DECODER_MUX_RS1(cpu);
            if (cpu->regs_state[cpu->decode.rs1] == 0)
            {
                cpu->regs_state[cpu->decode.rd] = 1;
                // rs1 is written back to in WB
                cpu->regs_state[cpu->decode.rs1] = 1;
                cpu->execute = cpu->decode;
                // cpu->execute.has_insn = TRUE;
                cpu->decode.has_insn = FALSE;
                cpu->stall_pipeline = 0;
            }
            else
            {
                cpu->stall_pipeline = 1;
            }
            break;
        }
        case OPCODE_STOREP:
        {
            FORWARDED_DECODER_MUX_RS1(cpu);
            FORWARDED_DECODER_MUX_RS2(cpu);

            if (cpu->regs_state[cpu->decode.rs1] == 0 && cpu->regs_state[cpu->decode.rs2] == 0) // dest regs have been updated
            {
                // r2 is written back to in WB - dest
                cpu->regs_state[cpu->decode.rs2] = 1;

                cpu->execute = cpu->decode;
                cpu->decode.has_insn = FALSE;
                cpu->stall_pipeline = 0;

                break;
            }
            else
            {
                cpu->stall_pipeline = 1;
            }
            break;
        }
        }

        // cpu->execute.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Decode/RF", &cpu->decode);
        }
    }
}

/*
 * Execute Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_execute(APEX_CPU *cpu)
{
    if (cpu->execute.has_insn)
    {
        /* Execute logic based on instruction type */
        switch (cpu->execute.opcode)
        {
        case OPCODE_ADD:
        {
            cpu->execute.result_buffer = cpu->execute.rs1_value + cpu->execute.rs2_value;

            // if ((cpu->execute.rd == cpu->decode.rs1) || (cpu->execute.rd == cpu->decode.rs2))
            // {
            //     cpu->decode.ex_forwarded = 1;
            //     cpu->decode.ex_tag_forwarded = cpu->execute.rd;
            //     cpu->decode.ex_value_forwarded = cpu->execute.result_buffer;
            //     cpu->regs_state[cpu->execute.rd] = 0;
            // }

            /* Set the zero flag based on the result buffer */
            if (cpu->execute.result_buffer == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            if (cpu->execute.result_buffer > 0)
            {
                cpu->positive_flag = TRUE;
            }
            else
            {
                cpu->positive_flag = FALSE;
            }
            if (cpu->execute.result_buffer < 0)
            {
                cpu->negative_flag = TRUE;
            }
            else
            {
                cpu->negative_flag = FALSE;
            }
            break;
        }

        case OPCODE_ADDL:
        {
            cpu->execute.result_buffer = cpu->execute.rs1_value + cpu->execute.imm;

            /* Set the zero flag based on the result buffer */
            if (cpu->execute.result_buffer == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            if (cpu->execute.result_buffer > 0)
            {
                cpu->positive_flag = TRUE;
            }
            else
            {
                cpu->positive_flag = FALSE;
            }
            if (cpu->execute.result_buffer < 0)
            {
                cpu->negative_flag = TRUE;
            }
            else
            {
                cpu->negative_flag = FALSE;
            }
            break;
        }

        case OPCODE_SUB:
        {
            cpu->execute.result_buffer = cpu->execute.rs1_value - cpu->execute.rs2_value;

            /* Set the zero flag based on the result buffer */
            if (cpu->execute.result_buffer == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            if (cpu->execute.result_buffer > 0)
            {
                cpu->positive_flag = TRUE;
            }
            else
            {
                cpu->positive_flag = FALSE;
            }
            if (cpu->execute.result_buffer < 0)
            {
                cpu->negative_flag = TRUE;
            }
            else
            {
                cpu->negative_flag = FALSE;
            }
            break;
        }

        case OPCODE_SUBL:
        {
            cpu->execute.result_buffer = cpu->execute.rs1_value - cpu->execute.imm;

            /* Set the zero flag based on the result buffer */
            if (cpu->execute.result_buffer == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            if (cpu->execute.result_buffer > 0)
            {
                cpu->positive_flag = TRUE;
            }
            else
            {
                cpu->positive_flag = FALSE;
            }
            if (cpu->execute.result_buffer < 0)
            {
                cpu->negative_flag = TRUE;
            }
            else
            {
                cpu->negative_flag = FALSE;
            }
            break;
        }
        case OPCODE_MUL:
        {
            cpu->execute.result_buffer = cpu->execute.rs1_value * cpu->execute.rs2_value;

            /* Set the zero flag based on the result buffer */
            if (cpu->execute.result_buffer == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            if (cpu->execute.result_buffer > 0)
            {
                cpu->positive_flag = TRUE;
            }
            else
            {
                cpu->positive_flag = FALSE;
            }
            if (cpu->execute.result_buffer < 0)
            {
                cpu->negative_flag = TRUE;
            }
            else
            {
                cpu->negative_flag = FALSE;
            }
            break;
        }
        case OPCODE_CMP:
        {
            int cmp_res = cpu->execute.rs1_value - cpu->execute.rs2_value;

            if (cmp_res == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            if (cmp_res > 0)
            {
                cpu->positive_flag = TRUE;
            }
            else
            {
                cpu->positive_flag = FALSE;
            }
            if (cmp_res < 0)
            {
                cpu->negative_flag = TRUE;
            }
            else
            {
                cpu->negative_flag = FALSE;
            }

            break;
        }

        case OPCODE_XOR:
        {
            cpu->execute.result_buffer = cpu->execute.rs1_value ^ cpu->execute.rs2_value;

            /* Set the zero flag based on the result buffer */
            if (cpu->execute.result_buffer == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            if (cpu->execute.result_buffer > 0)
            {
                cpu->positive_flag = TRUE;
            }
            else
            {
                cpu->positive_flag = FALSE;
            }
            if (cpu->execute.result_buffer < 0)
            {
                cpu->negative_flag = TRUE;
            }
            else
            {
                cpu->negative_flag = FALSE;
            }
            break;
        }

        case OPCODE_OR:
        {
            cpu->execute.result_buffer = cpu->execute.rs1_value | cpu->execute.rs2_value;

            /* Set the zero flag based on the result buffer */
            if (cpu->execute.result_buffer == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            if (cpu->execute.result_buffer > 0)
            {
                cpu->positive_flag = TRUE;
            }
            else
            {
                cpu->positive_flag = FALSE;
            }
            if (cpu->execute.result_buffer < 0)
            {
                cpu->negative_flag = TRUE;
            }
            else
            {
                cpu->negative_flag = FALSE;
            }
            break;
        }

        case OPCODE_AND:
        {
            cpu->execute.result_buffer = cpu->execute.rs1_value & cpu->execute.rs2_value;

            /* Set the zero flag based on the result buffer */
            if (cpu->execute.result_buffer == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            if (cpu->execute.result_buffer > 0)
            {
                cpu->positive_flag = TRUE;
            }
            else
            {
                cpu->positive_flag = FALSE;
            }
            if (cpu->execute.result_buffer < 0)
            {
                cpu->negative_flag = TRUE;
            }
            else
            {
                cpu->negative_flag = FALSE;
            }
            break;
        }
        case OPCODE_JALR:
        {
            cpu->execute.result_buffer = cpu->execute.pc + 4;

            /* Calculate new PC, and send it to fetch unit */
            cpu->pc = cpu->execute.rs1_value + cpu->execute.imm;

            /* Since we are using reverse callbacks for pipeline stages,
             * this will prevent the new instruction from being fetched in the current cycle*/
            cpu->fetch_from_next_cycle = TRUE;

            /* Flush previous stages */
            cpu->decode.has_insn = FALSE;

            /* Make sure fetch stage is enabled to start fetching from new PC */
            cpu->fetch.has_insn = TRUE;

            break;
        }

        case OPCODE_JUMP:
        {
            /* Calculate new PC, and send it to fetch unit */
            cpu->pc = cpu->execute.rs1_value + cpu->execute.imm;

            /* Since we are using reverse callbacks for pipeline stages,
             * this will prevent the new instruction from being fetched in the current cycle*/
            cpu->fetch_from_next_cycle = TRUE;

            /* Flush previous stages */
            cpu->decode.has_insn = FALSE;

            /* Make sure fetch stage is enabled to start fetching from new PC */
            cpu->fetch.has_insn = TRUE;

            break;
        }

        case OPCODE_CML:
        {
            int cml_res = cpu->execute.rs1_value - cpu->execute.imm;

            if (cml_res == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            if (cml_res > 0)
            {
                cpu->positive_flag = TRUE;
            }
            else
            {
                cpu->positive_flag = FALSE;
            }
            if (cml_res < 0)
            {
                cpu->negative_flag = TRUE;
            }
            else
            {
                cpu->negative_flag = FALSE;
            }

            break;
        }

        case OPCODE_LOAD:
        {
            cpu->execute.memory_address = cpu->execute.rs1_value + cpu->execute.imm;
            break;
        }

        case OPCODE_STORE:
        {
            cpu->execute.memory_address = cpu->execute.rs2_value + cpu->execute.imm;
            break;
        }

        case OPCODE_LOADP:
        {
            cpu->execute.memory_address = cpu->execute.rs1_value + cpu->execute.imm;
            cpu->execute.rs1_value = cpu->execute.rs1_value + 4;
            break;
        }

        case OPCODE_STOREP:
        {
            cpu->execute.memory_address = cpu->execute.rs2_value + cpu->execute.imm;
            cpu->execute.rs2_value = cpu->execute.rs2_value + 4;
            break;
        }

        case OPCODE_BZ:
        {
            if (cpu->zero_flag == TRUE)
            {
                /* Calculate new PC, and send it to fetch unit */
                cpu->pc = cpu->execute.pc + cpu->execute.imm;

                /* Since we are using reverse callbacks for pipeline stages,
                 * this will prevent the new instruction from being fetched in the current cycle*/
                cpu->fetch_from_next_cycle = TRUE;

                /* Flush previous stages */
                cpu->decode.has_insn = FALSE;

                /* Make sure fetch stage is enabled to start fetching from new PC */
                cpu->fetch.has_insn = TRUE;
            }
            break;
        }

        case OPCODE_BNZ:
        {
            if (cpu->zero_flag == FALSE)
            {
                /* Calculate new PC, and send it to fetch unit */
                cpu->pc = cpu->execute.pc + cpu->execute.imm;

                /* Since we are using reverse callbacks for pipeline stages,
                 * this will prevent the new instruction from being fetched in the current cycle*/
                cpu->fetch_from_next_cycle = TRUE;

                /* Flush previous stages */
                cpu->decode.has_insn = FALSE;

                /* Make sure fetch stage is enabled to start fetching from new PC */
                cpu->fetch.has_insn = TRUE;
            }
            break;
        }

        case OPCODE_BP:
        {
            if (cpu->positive_flag == TRUE)
            {
                /* Calculate new PC, and send it to fetch unit */
                cpu->pc = cpu->execute.pc + cpu->execute.imm;

                /* Since we are using reverse callbacks for pipeline stages,
                 * this will prevent the new instruction from being fetched in the current cycle*/
                cpu->fetch_from_next_cycle = TRUE;

                /* Flush previous stages */
                cpu->decode.has_insn = FALSE;

                /* Make sure fetch stage is enabled to start fetching from new PC */
                cpu->fetch.has_insn = TRUE;
            }
            break;
        }

        case OPCODE_BNP:
        {
            if (cpu->positive_flag == FALSE)
            {
                /* Calculate new PC, and send it to fetch unit */
                cpu->pc = cpu->execute.pc + cpu->execute.imm;

                /* Since we are using reverse callbacks for pipeline stages,
                 * this will prevent the new instruction from being fetched in the current cycle*/
                cpu->fetch_from_next_cycle = TRUE;

                /* Flush previous stages */
                cpu->decode.has_insn = FALSE;

                /* Make sure fetch stage is enabled to start fetching from new PC */
                cpu->fetch.has_insn = TRUE;
            }
            break;
        }

        case OPCODE_BN:
        {
            if (cpu->negative_flag == TRUE)
            {
                /* Calculate new PC, and send it to fetch unit */
                cpu->pc = cpu->execute.pc + cpu->execute.imm;

                /* Since we are using reverse callbacks for pipeline stages,
                 * this will prevent the new instruction from being fetched in the current cycle*/
                cpu->fetch_from_next_cycle = TRUE;

                /* Flush previous stages */
                cpu->decode.has_insn = FALSE;

                /* Make sure fetch stage is enabled to start fetching from new PC */
                cpu->fetch.has_insn = TRUE;
            }
            break;
        }

        case OPCODE_BNN:
        {
            if (cpu->negative_flag == FALSE)
            {
                /* Calculate new PC, and send it to fetch unit */
                cpu->pc = cpu->execute.pc + cpu->execute.imm;

                /* Since we are using reverse callbacks for pipeline stages,
                 * this will prevent the new instruction from being fetched in the current cycle*/
                cpu->fetch_from_next_cycle = TRUE;

                /* Flush previous stages */
                cpu->decode.has_insn = FALSE;

                /* Make sure fetch stage is enabled to start fetching from new PC */
                cpu->fetch.has_insn = TRUE;
            }
            break;
        }

        case OPCODE_MOVC:
        {
            cpu->execute.result_buffer = cpu->execute.imm;
            break;
        }
        }

        /* Copy data from execute latch to memory latch*/
        cpu->memory = cpu->execute;
        cpu->execute.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Execute", &cpu->execute);
        }
    }
}

/*
 * Memory Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_memory(APEX_CPU *cpu)
{
    if (cpu->memory.has_insn)
    {
        switch (cpu->memory.opcode)
        {
        case OPCODE_ADD:
        {
            /* No work for ADD */
            break;
        }

        case OPCODE_SUB:
        {
            /* No work for SUB */
            break;
        }

        case OPCODE_ADDL:
        {
            /* No work for ADD */
            break;
        }

        case OPCODE_SUBL:
        {
            /* No work for SUB */
            break;
        }

        case OPCODE_CMP:
        {
            /* No work for ADD */
            break;
        }

        case OPCODE_CML:
        {
            /* No work for SUB */
            break;
        }

        case OPCODE_NOP:
        {
            /* No work for ADD */
            break;
        }

        case OPCODE_LOAD:
        {
            /* Read from data memory */
            cpu->memory.result_buffer = cpu->data_memory[cpu->memory.memory_address];
            break;
        }
        case OPCODE_STORE:
        {
            cpu->data_memory[cpu->memory.memory_address] = cpu->memory.rs1_value;
            break;
        }
        case OPCODE_LOADP:
        {
            /* Read from data memory */
            cpu->memory.result_buffer = cpu->data_memory[cpu->memory.memory_address];
            break;
        }
        case OPCODE_STOREP:
        {
            cpu->data_memory[cpu->memory.memory_address] = cpu->memory.rs1_value;
            break;
        }

        case OPCODE_MOVC:
        {
            break;
        }
        }

        /* Copy data from memory latch to writeback latch*/
        cpu->writeback = cpu->memory;
        cpu->memory.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Memory", &cpu->memory);
        }
    }
}

/*
 * Writeback Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static int
APEX_writeback(APEX_CPU *cpu)
{
    if (cpu->writeback.has_insn)
    {
        /* Write result to register file based on instruction type */
        switch (cpu->writeback.opcode)
        {
        case OPCODE_ADD:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            cpu->regs_state[cpu->writeback.rd] = 0;
            break;
        }

        case OPCODE_ADDL:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            cpu->regs_state[cpu->writeback.rd] = 0;
            break;
        }

        case OPCODE_SUB:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            cpu->regs_state[cpu->writeback.rd] = 0;

            break;
        }

        case OPCODE_SUBL:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            cpu->regs_state[cpu->writeback.rd] = 0;

            break;
        }
        case OPCODE_MUL:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            cpu->regs_state[cpu->writeback.rd] = 0;
            break;
        }

        case OPCODE_LOAD:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            cpu->regs_state[cpu->writeback.rd] = 0;

            break;
        }

        case OPCODE_XOR:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            cpu->regs_state[cpu->writeback.rd] = 0;
            break;
        }

        case OPCODE_AND:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            cpu->regs_state[cpu->writeback.rd] = 0;
            break;
        }

        case OPCODE_OR:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            cpu->regs_state[cpu->writeback.rd] = 0;
            break;
        }

        case OPCODE_STORE:
        {
            break;
        }
        case OPCODE_LOADP:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            cpu->regs[cpu->writeback.rs1] = cpu->writeback.rs1_value;
            cpu->regs_state[cpu->writeback.rd] = 0;
            cpu->regs_state[cpu->writeback.rs1] = 0;

            break;
        }

        case OPCODE_STOREP:
        {
            cpu->regs[cpu->writeback.rs2] = cpu->writeback.rs2_value;
            cpu->regs_state[cpu->writeback.rs2] = 0;
            break;
        }

        case OPCODE_NOP:
        {
            break;
        }

        case OPCODE_MOVC:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            cpu->regs_state[cpu->writeback.rd] = 0;
            break;
        }

        case OPCODE_CMP:
        {
            // No destination register.
            break;
        }

        case OPCODE_CML:
        {
            // No destination register.
            break;
        }
        case OPCODE_JALR:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            cpu->regs_state[cpu->writeback.rd] = 0;
            break;
        }
        }

        cpu->insn_completed++;
        cpu->writeback.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Writeback", &cpu->writeback);
        }

        if (cpu->writeback.opcode == OPCODE_HALT)
        {
            /* Stop the APEX simulator */
            return TRUE;
        }
    }

    /* Default */
    return 0;
}

/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename)
{
    int i;
    APEX_CPU *cpu;

    if (!filename)
    {
        return NULL;
    }

    cpu = calloc(1, sizeof(APEX_CPU));

    if (!cpu)
    {
        return NULL;
    }

    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
    cpu->single_step = ENABLE_SINGLE_STEP;

    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
        fprintf(stderr,
                "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
                cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2",
               "imm");

        for (i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        }
    }

    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;
    return cpu;
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void APEX_cpu_run(APEX_CPU *cpu, int numCycles)
{
    char user_prompt_val;

    while (numCycles>0)
    {

        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock);
            printf("--------------------------------------------\n");
        }

        if (APEX_writeback(cpu))
        {
            /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
            break;
        }

        APEX_memory(cpu);
        APEX_execute(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);

        print_reg_file(cpu);
        print_data_memory(cpu); 
        print_flag_values(cpu);

        if (cpu->single_step)
        {
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                break;
            }
        }

        cpu->clock++;
        numCycles = numCycles-1; 
    }
}

/*
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void APEX_cpu_stop(APEX_CPU *cpu)
{
    free(cpu->code_memory);
    free(cpu);
}