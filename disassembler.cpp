#include <map>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <bitset>
#include <iomanip>
#include <string>
#include <fstream>

using namespace std;

struct item {
	int i, rs, rt, rd, imm, opcode, valid;
	string binStr, instrStr;
	unsigned int asUint;
};

ofstream dis;

string formatBinaryString(string inputString) {
	string formattedString = inputString.insert(26, 1, ' ');
	formattedString = formattedString.insert(21, 1, ' '); // Subtract by 5
	formattedString = formattedString.insert(16, 1, ' ');
	formattedString = formattedString.insert(11, 1, ' ');
	formattedString = formattedString.insert(6, 1, ' ');
	formattedString = formattedString.insert(1, 1, ' ');
	return formattedString;
}

void printInstruction(int address, item inputInstruction) {
	int instructionType = 0;
	if (inputInstruction.valid == 1) {
		switch (inputInstruction.opcode) {
			case 32:
				if ((inputInstruction.asUint << 26) >> 26 == 0) {
					inputInstruction.instrStr = "SLL\tR";
					instructionType = 4;
				} else if ((inputInstruction.asUint << 26) >> 26 == 2) {
					inputInstruction.instrStr = "SRL\tR";
					instructionType = 4;
				} else if (inputInstruction.imm == 8) {
					inputInstruction.instrStr.append("JR\tR" + to_string(inputInstruction.rs));
					instructionType = -1;
				} else if ((inputInstruction.asUint << 26) >> 26 == 10) {
					inputInstruction.instrStr = "MOVZ\tR";
					instructionType = 3;
				} else if ((inputInstruction.imm << 26) >> 26 == 13) {
					inputInstruction.instrStr = "BREAK";
					instructionType = -1;
				} else if ((inputInstruction.asUint << 26) >> 26 == 32) {
					inputInstruction.instrStr = "ADD\tR";
					instructionType = 3;
				} else if ((inputInstruction.asUint << 26) >> 26 == 34) {
					inputInstruction.instrStr = "SUB\tR";
					instructionType = 3;
				}
				break;
			case 33:
				inputInstruction.instrStr = "BLTZ\tR";
				instructionType = 2;
				break;
			case 34:
				inputInstruction.instrStr = "J\t";
				instructionType = 2;
				break;
			case 35:
				inputInstruction.instrStr = "LW\tR";
				instructionType = 1;
				break;
			case 40:
				inputInstruction.instrStr = "ADDI\tR";
				break;
			case 43:
				inputInstruction.instrStr = "SW\tR";
				instructionType = 1;
				break;
			case 60:
				inputInstruction.instrStr = "MUL\tR";
				instructionType = 3;
				break;
			default:
				break;
		}

		switch (instructionType) {
			case 0:
				inputInstruction.instrStr.append(to_string(inputInstruction.rt) + ", R"
				       + to_string(inputInstruction.rs) + ", #" + to_string(inputInstruction.imm));
				break;
			case 1:
				inputInstruction.instrStr.append(to_string(inputInstruction.rt) + ", " + to_string(inputInstruction.imm) + "(R"
					+ to_string(inputInstruction.rs) + ")");
				break;
			case 2:
				inputInstruction.imm = inputInstruction.imm << 2;
				if (inputInstruction.instrStr[0] == 'B') {
					inputInstruction.instrStr.append(to_string(inputInstruction.rs) + ", #" + to_string(inputInstruction.imm));
				} else {
					inputInstruction.instrStr.append("#" + to_string(inputInstruction.imm));
				}
				break;
			case 3:
				inputInstruction.instrStr.append(to_string((inputInstruction.asUint << 16) >> 27) + ", R" + to_string(inputInstruction.rs)
					+ ", R" + to_string(inputInstruction.rt));
				break;
			case 4:
				inputInstruction.instrStr.append(to_string((inputInstruction.asUint << 16) >> 27) + ", R" + to_string(inputInstruction.rt)
					+ ", #" + to_string((inputInstruction.asUint << 21) >> 27));
				break;
			default:
				break;
		}
	} else {
		inputInstruction.instrStr = "Invalid Instruction";
	}

	dis << inputInstruction.binStr << "\t" << address << "\t" << inputInstruction.instrStr << endl;
}

int main( int argc, char* argv[])
{
        char buffer[4];
        int i;
        char * iPtr;
        iPtr = (char*)(void*) &i;

        int FD = open(argv[2], O_RDONLY);
//        int FD = open("test1.bin", O_RDONLY);
	dis.open(argv[4], ofstream::out);
//	dis.open("test1_dis_local.txt", ofstream::out);

	map< int, item > MEM;
	int addr = 96;
        int amt = 4;
	bool hasHitBreak = false;
        while( amt != 0 )
        {
                amt = read(FD, buffer, 4);
                if( amt == 4 )
                {
                        iPtr[0] = buffer[3];
                        iPtr[1] = buffer[2];
                        iPtr[2] = buffer[1];
                        iPtr[3] = buffer[0];
                	item instruction;
			unsigned int asUint = (unsigned int) i;
			instruction.asUint = asUint;
			bitset<32> b( i );
			instruction.binStr = b.to_string();
			instruction.valid = asUint >> 31;
			instruction.opcode = asUint >> 26;
			instruction.rs = (asUint << 6) >> 27;
			instruction.rt = (asUint << 11) >> 27;
			instruction.imm = (i << 16) >> 16;
			//cout << "valid bit: " << valid << endl;
			//cout << "opcode: " << instruction.opcode << endl;
			//cout << binstr << "\t"; 
			if(!hasHitBreak){
				instruction.binStr = formatBinaryString(instruction.binStr);
				printInstruction(addr, instruction);
				if (instruction.imm == 13) { hasHitBreak = true; }
			} else {
				dis << instruction.binStr << "\t" << addr << "\t" << instruction.imm << endl;
			}
			MEM[addr] = instruction;
			addr+=4;
		}
        } // end of decode
	cout << endl;
	dis.close();
/*
	// start sim
	int PC = 96;
	int R[32] = {0};
	int cycle = 1;

	while( true ){
		item instruction = MEM[PC];
		while( instruction.valid == 0 ){
			PC +=4;
			instruction = MEM[PC];
		}
		if(instruction.opcode == 40 ) {
			R[instruction.rt] = R[instruction.rs] + instruction.imm;
		}
		cout << "====================\ncycle:" + to_string(cycle) 
			+ " " + to_string(PC) +"\t" + instruction.instrStr + " " + "\n\nregisters:\n"
			+ to_string(R[0]) + " " + to_string(R[1]) +"\n";

		PC += 4;
		cycle ++;

		if( cycle >= 2) break;
	}
*/
}
