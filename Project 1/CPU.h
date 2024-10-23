#ifndef CPU_H_
#define CPU_H_

#include <iostream>
#include <bitset>
#include <stdio.h>
#include<stdlib.h>
#include <string>
using namespace std;


class Instruction { //32 bit string
public:
	bitset<32> instr;//instruction
	Instruction(bitset<32> fetch) : instr(fetch) {}; // constructor

};

class CPU {
public:
	CPU();
	unsigned long readPC();
	void incPC();

private:
	int dmemory[4096]; //data memory byte addressable in little endian fashion;
	unsigned long PC; //pc 

};

// add other functions and objects here

#endif