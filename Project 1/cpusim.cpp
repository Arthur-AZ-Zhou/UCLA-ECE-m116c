#include "CPU.h"

#include <iostream>
#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;

string fetchedAscii(uint32_t tempPC, char instMem[]) {
	string byte1 = "";
	string byte2 = "";
	string byte3 = "";
	string byte4 = "";

	for (uint32_t i = 0; i < 2; i++) {
		byte1 += instMem[tempPC + i];
	}

	for (uint32_t i = 2; i < 4; i++) {
		byte2 += instMem[tempPC + i];
	}

	for (uint32_t i = 4; i < 6; i++) {
		byte3 += instMem[tempPC + i];
	}

	for (uint32_t i = 6; i < 8; i++) {
		byte4 += instMem[tempPC + i];
	}
	
	// cout << "byte4: " << byte4 << endl;
	// cout << "byte3: " << byte3 << endl;
	// cout << "byte2: " << byte2 << endl;
	// cout << "byte1: " << byte1 << endl;
	// cout << byte4 + byte3 + byte2 + byte1 << endl;
	return byte4 + byte3 + byte2 + byte1;
}

bitset<32> hexDecoder(string asciiHexInput) {
	stringstream ss;
	uint32_t hexNum;

	ss << hex << asciiHexInput;
	ss >> hexNum;
	bitset<32> binaryString(hexNum);

	return binaryString;
}

int main(int argc, char* argv[]) {
	char instMem[4096];

	//ERROR HANDLING--------------------------------------------------------------------------------------------------------------------------
	if (argc < 2) {
		cout << "No file name entered. Exiting..." << endl;
		return -1;
	}

	ifstream infile(argv[1]); //open the file
	if (!(infile.is_open() && infile.good())) {
		cout<<"error opening file\n";
		return 0; 
	}

	//READING EACH LINE-----------------------------------------------------------------------------------------------------------------------
	string line; 
	int i = 0;
	while (infile >> line) { //GOES THRU EACH LINE IN BYTE FILE
		if (line.length() != 2) { //skip any invalid lines
			break;
		}

		stringstream line2(line); //splits line into two characters, like 93 = 9, 3
		char x; 
		line2>>x;
		instMem[i] = x; // be careful about hex, IT WILL BE IN ASCII IN instMem[]
		i++;
		line2>>x;
		instMem[i] = x; // be careful about hex
		// cout<<instMem[i]<<endl;
		i++;
	}
	int maxPC= i/8; 

	//ACTUALLY WORKING------------------------------------------------------------------------------------------------------------------------
	bitset<32> tempInstr("11010010101100101001001010100101"); //just so compiler is happy

	CPU myCPU;  
	Instruction myInst(tempInstr); //we're gonna replace this anyways lmao
	bool done = true;

	while (done == true) { // processor's main loop. Each iteration is equal to one clock cycle.  
		//fetch--------------------------------------------------------------------------------
		uint32_t tempPC = myCPU.readPC() * 8;
		string fetched = fetchedAscii(tempPC, instMem);

		if (fetched.length() != 8) {
			break;
		}
		cout << "fetched: " << fetched << endl;

		myInst.instr = hexDecoder(fetched);
		cout << "32-bit decoded instr: " << myInst.instr.to_string() << endl;

		//decode------------------------------------------------------------------------------
		

		//execute-----------------------------------------------------------------------------
		
		//cleanup-----------------------------------------------------------------------------
		myCPU.incPC();
		if (myCPU.readPC() >= maxPC)
			break;
	}
	int a0 = 0; //myCPU regfile[10]
	int a1 = 0;  //myCPU regfile[11]
	// print the results (you should replace a0 and a1 with your own variables that point to a0 and a1)
	cout << "(" << a0 << "," << a1 << ")" << endl;
	
	return 0;
}