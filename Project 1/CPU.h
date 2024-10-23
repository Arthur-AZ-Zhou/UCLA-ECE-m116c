#ifndef CPU_H_
#define CPU_H_

#include <iostream>
#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>
using namespace std;

enum types {
	Rtype, Itype, Stype, Btype, Utype, Jtype
};

enum op {
	ADD, LUI, ORI, XOR, SRAI, LB, LW, SB, SW, BEQ, JAL
};

extern unordered_map<string, types> opcodeMap;

class Instruction { //32 bit string
public:
	bitset<32> instr;//instruction
	Instruction(bitset<32> fetch) : instr(fetch) {}; // constructor
};

class CPU {
public:
	CPU();
	unsigned long readPC() { return PC; };
	void incPC() { PC++; };
	void decode(Instruction* instruction);
	void execute();

	types getOpcode() { return opcode; };
	op getOperation() { return operation; };
	int32_t get_a0() { return regfile[10]; };
	int32_t get_a1() { return regfile[11]; };

private:
	int dmemory[4096]; //data memory byte addressable in little endian fashion;
	int32_t regfile[32]; // x0 - x31 are each 4 bytes
	unsigned long PC; //pc 

	types opcode;
	op operation;
	uint32_t funct7;
    uint32_t funct3;
	uint32_t rs1;
	uint32_t rs2;
	uint32_t rd;
	uint32_t imm;
	uint32_t shamt;
	uint32_t aluRes;
};

// add other functions and objects here

#endif