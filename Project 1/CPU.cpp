#include "CPU.h"

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
}

void CPU::decode(Instruction* instruction) {
    string bitString32 = instruction->instr.to_string();
    string opcodeSubstr = bitString32.substr(25);
    cout << "opcode: " << opcodeSubstr << endl;

    if (opcodeMap.find(opcodeSubstr) == opcodeMap.end()) {
        cout << "opcode not found!" << endl;
        return;
    } else {
        opcode = opcodeMap[opcodeSubstr];
    }

    switch (opcode) {
        case Rtype:
            cout << "Opcode Type is R-type." << endl;
            funct7 = bitset<7>(bitString32.substr(0, 7)).to_ulong();
            rs2 = bitset<5>(bitString32.substr(7, 5)).to_ulong();  
            rs1 = bitset<5>(bitString32.substr(12, 5)).to_ulong();      
            funct3 = bitset<3>(bitString32.substr(17, 3)).to_ulong(); 
            rd = bitset<5>(bitString32.substr(20, 5)).to_ulong();
            cout << "funct 7: " << funct7 << " rs2: " << rs2 << " rs1: " << rs1 << " funct3: " << funct3 << " rd: " << rd << endl;

            if (funct3 == 0 && funct7 == 0) { //ADD
                cout << "opearation: ADD" << endl;
                operation = ADD;
            } else if (funct3 == 4 && funct7 == 0) { //XOR
                cout << "operation: XOR" << endl;
                operation = XOR;
            } else { //ERROR
                cout << "error! Not a valid funct3!" << endl;
            }

            break;
        case Itype:
            std::cout << "Opcode Type is I-type." << std::endl;
            imm = bitset<12>(bitString32.substr(0, 12)).to_ulong(); 
            rs1 = bitset<5>(bitString32.substr(12, 5)).to_ulong();    
            funct3 = bitset<3>(bitString32.substr(17, 3)).to_ulong(); 
            rd = bitset<5>(bitString32.substr(20, 5)).to_ulong();
            funct7 = bitset<7>(bitString32.substr(0, 7)).to_ulong();
            cout << "imm: " << imm << " rs1: " << rs1 << " funct3: " << funct3 << " rd: " << rd << endl;

            if (opcodeSubstr == "0010011") { //could be LB or LW
                if (funct3 == 6) { //ORI
                    cout << "operation: ORI" << endl;
                    operation = ORI;
                } else if (funct3 == 5 && funct7 == 0b0100000) { //SRAI
                    cout << "operation: SRAI" << endl;
                    operation = SRAI;
                } else {
                    cout << "error, not valid!" << endl;
                }
            } else if (opcodeSubstr == "0000011") { //ORI or SRAI
                if (funct3 == 0) { //LB
                    cout << "operation: LB" << endl;
                    operation = LB;
                } else if (funct3 == 2) { //LW
                    cout << "operation: LW" << endl;
                    operation = LW;
                } else {
                    cout << "error, not valid!" << endl;
                }
            } else {
                cout << "error, not valid!" << endl;
            }

            break;
        case Stype:
            std::cout << "Opcode Type is S-type." << std::endl;
            

            break;
        case Btype:
            std::cout << "Opcode Type is B-type." << std::endl;
            break;
        case Utype:
            std::cout << "Opcode Type is U-type." << std::endl;
            break;
        case Jtype:
            std::cout << "Opcode Type is J-type." << std::endl;
            break;
        default:
            std::cout << "Unknown Opcode Type." << std::endl;
            break;
    }
}

// Add other functions here ... 