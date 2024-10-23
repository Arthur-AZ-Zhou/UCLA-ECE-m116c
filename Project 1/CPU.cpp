#include "CPU.h"
#include <cstring>

unordered_map<string, types> opcodeMap = {
	{"0110011", Rtype},
	{"0010011", Itype},
	{"0000011", Itype},
	{"1110011", Itype},
	{"0100011", Stype},
	{"1100011", Btype},
	{"0110111", Utype},
	{"0010111", Utype},
	{"1101111", Jtype}
};

CPU::CPU() {
	PC = 0; //set PC to 0
	for (int i = 0; i < 4096; i++) {//copy instrMEM
		dmemory[i] = (0);
	}

    memset(regfile, 0, sizeof(regfile));
}

void CPU::decode(Instruction* instruction) {
    string bitString32 = instruction->instr.to_string();
    unsigned long bitVal = (instruction->instr).to_ulong();
    string opcodeSubstr = bitString32.substr(25);
    // cout << "opcode: " << opcodeSubstr << endl;

    if (opcodeMap.find(opcodeSubstr) == opcodeMap.end()) {
        // cout << "opcode not found!" << endl;
        return;
    } else {
        opcode = opcodeMap[opcodeSubstr];
    }

    uint32_t imm1;
    uint32_t imm2;
    uint32_t imm3;
    uint32_t imm4;
    uint32_t imm5;

    switch (opcode) {
        case Rtype:
            // cout << "Opcode Type is R-type." << endl;
            funct7 = bitset<7>(bitString32.substr(0, 7)).to_ulong();
            rs2 = bitset<5>(bitString32.substr(7, 5)).to_ulong();  
            rs1 = bitset<5>(bitString32.substr(12, 5)).to_ulong();      
            funct3 = bitset<3>(bitString32.substr(17, 3)).to_ulong(); 
            rd = bitset<5>(bitString32.substr(20, 5)).to_ulong();
            // cout << "funct 7: " << funct7 << " rs2: " << rs2 << " rs1: " << rs1 << " funct3: " << funct3 << " rd: " << rd << endl;

            if (funct3 == 0 && funct7 == 0) { //ADD
                // cout << "opearation: ADD" << endl;
                operation = ADD;
            } else if (funct3 == 4 && funct7 == 0) { //XOR
                // cout << "operation: XOR" << endl;
                operation = XOR;
            } else { //ERROR
                // cout << "error! Not a valid funct3!" << endl;
            }

            break;
        case Itype:
            // std::cout << "Opcode Type is I-type." << std::endl;
            imm = bitset<12>(bitString32.substr(0, 12)).to_ulong(); 
            rs1 = bitset<5>(bitString32.substr(12, 5)).to_ulong();    
            funct3 = bitset<3>(bitString32.substr(17, 3)).to_ulong(); 
            rd = bitset<5>(bitString32.substr(20, 5)).to_ulong();
            funct7 = bitset<7>(bitString32.substr(0, 7)).to_ulong();
            // cout << "imm: " << imm << " rs1: " << rs1 << " funct3: " << funct3 << " rd: " << rd << endl;

            if (opcodeSubstr == "0010011") { //could be LB or LW
                if (funct3 == 6) { //ORI
                    // cout << "operation: ORI" << endl;
                    operation = ORI;
                } else if (funct3 == 5 && funct7 == 0b0100000) { //SRAI
                    // cout << "operation: SRAI" << endl;
                    operation = SRAI;
                } else {
                    // cout << "error, not valid!" << endl;
                }
            } else if (opcodeSubstr == "0000011") { //ORI or SRAI
                if (funct3 == 0) { //LB
                    // cout << "operation: LB" << endl;
                    operation = LB;
                } else if (funct3 == 2) { //LW
                    // cout << "operation: LW" << endl;
                    operation = LW;
                } else {
                    // cout << "error, not valid!" << endl;
                }
            } else {
                // cout << "error, not valid!" << endl;
            }

            break;
        case Stype:
            // cout << "Opcode Type is S-type." << endl;
            imm1 = bitset<7>(bitString32.substr(0, 7)).to_ulong(); 
            rs2 = bitset<5>(bitString32.substr(7, 5)).to_ulong();  
            rs1 = bitset<5>(bitString32.substr(12, 5)).to_ulong(); 
            funct3 = bitset<3>(bitString32.substr(17, 3)).to_ulong(); 
            imm2 = bitset<5>(bitString32.substr(20, 5)).to_ulong(); 
            imm = (imm1 << 5) | imm2;
			// cout << "imm1: " << imm1 << " rs2: " << rs2 << " rs1: " << rs1 << " funct3: " << funct3 << " imm2: " << imm2 << " imm: " << imm << endl;

            if (funct3 == 0) {
                // cout << "operation: SB" << endl;
                operation = SB;
            } else if (funct3 == 2) {
                // cout << "operation: SW" << endl;
                operation = SW;
            } else {
                // cout << "error, not valid!" << endl;
            }

            break;
        case Btype:
            // cout << "Opcode Type is B-type." << endl;  
            // rs2 = bitset<5>(bitString32.substr(7, 5)).to_ulong();            
            // rs1 = bitset<5>(bitString32.substr(12, 5)).to_ulong();     
            // funct3 = bitset<3>(bitString32.substr(17, 3)).to_ulong();   
            // imm1 = (int32_t)(0);
            // imm2 = (int32_t)((bitVal & 0xf00) >> 7); //bits 11 to 7
            // imm3 = (int32_t)((bitVal & 0x7e000000) >> 20); // bits 31 to 25
            // imm4 = (int32_t)((bitVal & 0x80) << 4); // bit 24
            // imm5 = (int32_t)((bitVal & 0x80000000) >> 19); // bit 31
            // imm = (int32_t)(imm1 + imm2 + imm3 + imm4 + imm5);
            // if (imm5) {
            //     imm |= 0xfffff000;
            // }
			// cout << "imm1: " << imm1 << " imm2: " << imm2 << " rs2: " << rs2 << " rs1: " << rs1 << " funct3: " << funct3 << " imm3: " << imm3 << " imm4: " << imm4;
            // cout << " imm: " << imm << endl;

            // if (funct3 == 0) {
            //     cout << "operation: BEQ" << endl;
            //     operation = BEQ;
            // } else {
            //     cout << "error, not valid!" << endl;
            // }

            // cout << "Opcode Type is B-type." << endl;
            imm1 = bitset<1>(bitString32.substr(0, 1)).to_ulong();    
            imm2 = bitset<6>(bitString32.substr(1, 6)).to_ulong();   
            rs2 = bitset<5>(bitString32.substr(7, 5)).to_ulong();            
            rs1 = bitset<5>(bitString32.substr(12, 5)).to_ulong();     
            funct3 = bitset<3>(bitString32.substr(17, 3)).to_ulong();   
            imm3 = bitset<4>(bitString32.substr(20, 4)).to_ulong();      
            imm4 = bitset<1>(bitString32.substr(24, 1)).to_ulong();       
            imm = (imm1 << 11) | (imm2 << 5) | (imm3 << 1) | imm4;  
			// cout << "imm1: " << imm1 << " imm2: " << imm2 << " rs2: " << rs2 << " rs1: " << rs1 << " funct3: " << funct3 << " imm3: " << imm3 << " imm4: " << imm4;
            // cout << " imm: " << imm << endl;

            if (funct3 == 0) {
                // cout << "operation: BEQ" << endl;
                operation = BEQ;
            } else {
                // cout << "error, not valid!" << endl;
            }

            break;
        case Utype:
            // cout << "Opcode Type is U-type." << endl;
            imm = bitset<20>(bitString32.substr(0, 20)).to_ulong();
			rd = bitset<5>(bitString32.substr(20, 5)).to_ulong();
			// cout << "imm: " << imm << " rd " << rd << endl;

            if (opcodeSubstr == "0110111") {
                // cout << "operation: LUI" << endl;
                operation = LUI;
            } else {
                // cout << "error, not valid!" << endl;
            }

            break;
        case Jtype:
            // cout << "Opcode Type is J-type." << endl;
            // imm1 = (int32_t)((bitVal & 0x80000000) >> 11);
            // imm2 = (int32_t)((bitVal & 0x7fe00000) >> 20);
            // imm3 = (int32_t)((bitVal & 0x100000) >> 9);
            // imm4 = (int32_t)((bitVal & 0xff000));
			// rd = bitset<5>(bitString32.substr(20, 5)).to_ulong();
            // imm = imm1 | imm2 | imm3 | imm4;
            // imm = (imm << 11) >> 11;
			// cout << "imm1: " << imm1 << " imm2: " << imm2 << " imm3: " << imm3 << " imm4: " << imm4 << " rd: " << rd << " imm: " << imm << endl;

            // if (opcodeSubstr == "1101111") {
            //     cout << "operation: JAL" << endl;
            //     operation = JAL;
            // } else {
            //     cout << "error, not valid!" << endl;
            // }

            // cout << "Opcode Type is J-type." << endl;
            imm1 = bitset<1>(bitString32.substr(0, 1)).to_ulong();	
			imm2 = bitset<10>(bitString32.substr(1, 10)).to_ulong(); 
			imm3 = bitset<1>(bitString32.substr(11, 1)).to_ulong();
			imm4 = bitset<8>(bitString32.substr(12, 8)).to_ulong(); 
			rd = bitset<5>(bitString32.substr(20, 5)).to_ulong();
            imm = (imm1 << 20) | (imm2 << 1) | (imm3 << 11) | imm4;
			// cout << "imm1: " << imm1 << " imm2: " << imm2 << " imm3: " << imm3 << " imm4: " << imm4 << " rd: " << rd << " imm: " << imm << endl;

            if (opcodeSubstr == "1101111") {
                // cout << "operation: JAL" << endl;
                operation = JAL;
            } else {
                // cout << "error, not valid!" << endl;
            }

            break;
        default:
            // cout << "Unknown Opcode Type." << endl;
            break;
    }
}

void CPU::execute() {
    // cout << "Current PC: " << std::hex << (int) (PC * 4) << endl;
    // cout << std::dec;

    switch (operation) {
        case ADD:
            aluRes = regfile[rs1] + regfile[rs2];
            regfile[rd] = aluRes;
            incPC();
            // printRegisters();
            break;

        case XOR:
            aluRes = regfile[rs1] ^ regfile[rs2];
            regfile[rd] = aluRes;
            incPC();
            // printRegisters();
            break;

        case ORI:
            aluRes = regfile[rs1] | imm;
            regfile[rd] = aluRes;
            incPC();
            // printRegisters();
            break;

        case SRAI:
            aluRes = static_cast<int32_t>(regfile[rs1]) >> imm;
            regfile[rd] = aluRes;
            incPC();
            // printRegisters();
            break;

        case LB:
            regfile[rd] = (int32_t) (int8_t) dmemory[regfile[rs1] + imm];
            incPC();
            // printRegisters();
            break;

        case LW:
            // regfile[rd] = *reinterpret_cast<int32_t*>(&dmemory[regfile[rs1] + imm]);
            regfile[rd] = dmemory[regfile[rs1] + imm];
            incPC();
            // printRegisters();
            break;

        case SB:
            dmemory[regfile[rs1] + imm] = regfile[rs2] & 0xff;
            incPC();
            // printRegisters();
            break;

        case SW:
            // *reinterpret_cast<int32_t*>(&dmemory[regfile[rs1] + imm]) = regfile[rs2];
            dmemory[regfile[rs1] + imm] = regfile[rs2];
            incPC();
            // printRegisters();
            break;

        case BEQ:
            if (regfile[rs1] == regfile[rs2]) {
                // cout << "EQUAL SO BRANCH" << endl;
                PC += (imm/4);
            } else {
                // cout << "DO NOT BRANCH NOT EQUAL" << endl;
                incPC();
            }
            // printRegisters();
            break;

        case LUI:
            regfile[rd] = imm << 12;
            incPC();
            // printRegisters();
            break;

        case JAL:
            regfile[rd] = (PC + 1) * 4; //saves return addy
            PC += (imm/4);
            // printRegisters();
            break;

        default:
            // cout << "idk how you even got here..." << endl;
            break;
    }
}

// void CPU::printRegisters() {
//     cout << "Register Vals: " << endl;
//     for (int32_t i = 0; i < 32; ++i) {
//         std::cout << "| x" << i << ": " << regfile[i] << " ";
//     }
//     cout << endl;
// }