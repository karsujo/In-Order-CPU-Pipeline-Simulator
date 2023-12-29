/*
 * main.c
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>

#include "apex_cpu.h"

int
main(int argc, char const *argv[])
{
    APEX_CPU *cpu;
    int cycles;

    fprintf(stderr, "APEX CPU Pipeline Simulator\n");

    if(argc==2)
    {
        cycles = 5000; 
    }
    else if(argc==4)
    {
        cycles =  atoi(argv[3]); 
    }
    else
    {
        fprintf(stderr, "APEX_Help: Usage %s <input_file> OR %s <input_file> simulate <n>\n", argv[0],argv[0]);
        exit(1);
    }

    cpu = APEX_cpu_init(argv[1]);

    if (!cpu)
    {
        fprintf(stderr, "APEX_Error: Unable to initialize CPU\n");
        exit(1);
    }
    APEX_cpu_run(cpu, cycles);
    APEX_cpu_stop(cpu);
    return 0;
}