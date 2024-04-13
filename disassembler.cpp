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

int PC = 96;
int R[32] = {0};
int cycle = 1;
int preissue[4] = {0};
int premem[2] = {0};
int prealu[2] = {0};
int postmem = 0;
int postalu = 0;
bool didBreak = false;
map< int, item> MEM;
ofstream dis;
ofstream sim;

string formatBinaryString(string inputString) {
	string formattedString = inputString.insert(26, 1, ' ');
	formattedString = formattedString.insert(21, 1, ' '); // Subtract by 5
	formattedString = formattedString.insert(16, 1, ' ');
	formattedString = formattedString.insert(11, 1, ' ');
	formattedString = formattedString.insert(6, 1, ' ');
	formattedString = formattedString.insert(1, 1, ' ');
	return formattedString;
}

void printInstruction(int address, item* inputInstruction) {
	int instructionType = 0;
	int bottomSixBits = ((inputInstruction->asUint << 26) >> 26);
	if (inputInstruction->valid == 1) {
		switch (inputInstruction->opcode) {
			case 32:
				if ( bottomSixBits == 0) {
					if (inputInstruction->rt > 0) {
						inputInstruction->instrStr = "SLL\tR";
						instructionType = 4;
					} else {
						inputInstruction->instrStr = "NOP";
						instructionType = -1;
					}
				} else if ( bottomSixBits == 2) {
					inputInstruction->instrStr = "SRL\tR";
					instructionType = 4;
				} else if (inputInstruction->imm == 8) {
					inputInstruction->instrStr.append("JR\tR" + to_string(inputInstruction->rs));
					instructionType = -1;
				} else if (  bottomSixBits  == 10) {
					inputInstruction->instrStr = "MOVZ\tR";
					instructionType = 3;
				} else if ( bottomSixBits  == 13) {
					inputInstruction->instrStr = "BREAK";
					instructionType = -1;
				} else if ( bottomSixBits == 32) {
					inputInstruction->instrStr = "ADD\tR";
					instructionType = 3;
				} else if ( bottomSixBits == 34) {
					inputInstruction->instrStr = "SUB\tR";
					instructionType = 3;
				}
				break;
			case 33:
				inputInstruction->instrStr = "BLTZ\tR";
				instructionType = 2;
				break;
			case 34:
				inputInstruction->instrStr = "J\t";
				instructionType = 2;
				break;
			case 35:
				inputInstruction->instrStr = "LW\tR";
				instructionType = 1;
				break;
			case 40:
				inputInstruction->instrStr = "ADDI\tR";
				break;
			case 43:
				inputInstruction->instrStr = "SW\tR";
				instructionType = 1;
				break;
			case 60:
				inputInstruction->instrStr = "MUL\tR";
				instructionType = 3;
				break;
			default:
				break;
		}

		switch (instructionType) {
			case 0:
				inputInstruction->instrStr.append(to_string(inputInstruction->rt) + ", R"
				       + to_string(inputInstruction->rs) + ", #" + to_string(inputInstruction->imm));
				break;
			case 1:
				inputInstruction->instrStr.append(to_string(inputInstruction->rt) + ", " + to_string(inputInstruction->imm) + "(R"
					+ to_string(inputInstruction->rs) + ")");
				break;
			case 2:
				inputInstruction->imm = inputInstruction->imm << 2;
				if (inputInstruction->instrStr[0] == 'B') {
					inputInstruction->instrStr.append(to_string(inputInstruction->rs) + ", #" + to_string(inputInstruction->imm));
				} else {
					inputInstruction->instrStr.append("#" + to_string(inputInstruction->imm));
				}
				break;
			case 3:
				inputInstruction->instrStr.append(to_string((inputInstruction->asUint << 16) >> 27) + ", R" + to_string(inputInstruction->rs)
					+ ", R" + to_string(inputInstruction->rt));
				break;
			case 4:
				inputInstruction->instrStr.append(to_string((inputInstruction->asUint << 16) >> 27) + ", R" + to_string(inputInstruction->rt)
					+ ", #" + to_string((inputInstruction->asUint << 21) >> 27));
				break;
			default:
				break;
		}
	} else {
		inputInstruction->instrStr = "Invalid Instruction";
	}

	dis << inputInstruction->binStr << "\t" << address << "\t" << inputInstruction->instrStr << endl;
}

string getQueues() {
	string message;
	message += "\nPre-Issue Buffer:\n";
	for (int i = 0; i < 4; i++ ) {
		message += "\tEntry " + to_string(i) + ":\t" + MEM[preissue[i]].instrStr + "\n";
	}

	message += "Pre_ALU Queue:\n";
	for (int i = 0; i < 2; i++) {
		message += "\tEntry " + to_string(i) + ":\t" + MEM[prealu[i]].instrStr + "\n";
	}

	message += "Post_ALU Queue:\n\tEntry 0:\t" + MEM[postalu].instrStr + "\n";

	message += "Pre_MEM Queue:\n";
	for (int i = 0; i < 2; i++) {
		message += "\tEntry " + to_string(i) + ":\t" + MEM[premem[i]].instrStr + "\n";
	}

	message += "Post_MEM Queue:\n\tEntry 0:\t" + MEM[postmem].instrStr + "\n\n";

	return message;
}

string getRegisters() {
	string message = "Registers";
	for (int i = 0; i < 32; i++) {
		if (i % 8 == 0) {
			message += "\nR" + ((i < 15) ? (to_string(0) + to_string(i)) : (to_string(i))) + ":\t";
		}
		message += to_string(R[i]) + "\t";
	}
	return message;
}

int main( int argc, char* argv[])
{
        char buffer[4];
        int i;
        char * iPtr;
        iPtr = (char*)(void*) &i;

	int FD;
	if (argc > 3) {
		string inputName = argv[2];
		inputName.append(".bin");
		FD = open(inputName.c_str(), O_RDONLY);
	} else {
		FD = open("test1.bin", O_RDONLY);
	}
	// int FD = open(inputName, O_RDONLY);
	dis.open((string(argv[4]) + "_dis.txt").c_str(), ofstream::out);

	// map< int, item > MEM;
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
				// cout << "valid bit: " << valid << endl;
				// cout << "opcode: " << instruction.opcode << endl;
				// cout << binstr << "\t"; 
				if(!hasHitBreak){
					instruction.binStr = formatBinaryString(instruction.binStr);
					printInstruction(addr, &instruction);
					if (instruction.imm == 13) { hasHitBreak = true; }
				} else {
					dis << instruction.binStr << "\t" << addr << "\t" << instruction.imm << endl;
				}
				MEM[addr] = instruction;
				// cout << MEM.at(addr).instrStr << endl;
				addr+=4;
			}
        } // end of decode
	cout << endl;
	dis.close();

	sim.open((string(argv[4]) + "_pipeline.txt").c_str(), ofstream::out);
	// start sim
	// int PC = 96;
	// int R[32] = {0};
	int cycle = 1;

	bool hasHitMemoryBreak = false;
	while( !hasHitMemoryBreak ){
		item instruction = MEM[PC];
		if ( instruction.valid == 0 ){
			PC +=4;
			continue;
		}

		switch (instruction.opcode) {
			case 32:
				break;
			case 33:
				break;
			case 34:
				break;
			case 35:
				break;
			case 40:
				R[instruction.rt] = R[instruction.rs] + instruction.imm;
				break;
			case 42:
				break;
			case 46:
				break;
			case 60:
				break;
		}

		preissue[0] = PC;

		// string output = "====================\ncycle:" + to_string(cycle) + " " + to_string(PC) + "\t" + instruction.instrStr + "\n\nregisters:";
		string output = "--------------------\nCycle:" + to_string(cycle) + "\n";

		output += getQueues();

		output += getRegisters();

		output += "\n\nData";

		for (int i = 0; i < 24; i++) {
			if (i % 8 == 0) {
				int memoryAddress = 172 + (i * 4);
				output += "\n" + to_string(memoryAddress) + ":\t";
			}
			output += (to_string(instruction.imm) + "\t");
		}
		output += "\n\n";

		sim << output;
/**/
		PC += 4;
		cycle ++;

		if( instruction.instrStr == "BREAK" ) hasHitMemoryBreak = true;
	}
	sim.close();
}
