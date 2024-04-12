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
	int i, rs, rt, rd, imm, opcode, valid,
		target, func, samt, jTarget, dest, src1, src2;
	string binStr, instrStr, binStrSpace;
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

void printQueues() {

}

void runSimulation(item* inputInstruction) {
	string output = "--------------------\nCycle: " + to_string(cycle) + "\n" + to_string(PC) + "\t" + instruction.instrStr + "\n\nregisters:";

	for (int i = 0; i < 32; i++) {
		if (i % 8 == 0) {
			output += "\nr" + ((i < 15) ? (to_string(0) + to_string(i)) : (to_string(i))) + ":\t";
		}
		output += to_string(R[i]) + "\t";
	}
	output += "\n\ndata:";

	for (int i = 0; i < 24; i++) {
		if (i % 8 == 0) {
			int memoryAddress = 172 + (i * 4);
			output += "\n" + to_string(memoryAddress) + ":\t";
		}
		output += (to_string(instruction.imm) + "\t");
	}
	output += "\n\n";

	sim << output;
}

bool XBW( int rNum, int index ) {
	for( int i = 0; i < index; i++ ) {
		if( preissue[i] !=0 && MEM[preissue[i]].dest == rNum) return true;
	}	
	for( int i = 0; i < 2; i++ ) {
		if( premem[i] != 0 &&MEM[premem[i]].dest == rNum) return true;
	}	
	for( int i = 0; i < 2; i++ ) {
		if( prealu[i] != 0 && MEM[prealu[i]].dest == rNum) return true;
	}	
	if( postalu != 0 && MEM[postalu].dest == rNum) return true;
	if( postmem !=0 && MEM[postmem].dest == rNum) return true;
	return false;
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
				addr+=4;
			}
        } // end of decode
	cout << endl;
	dis.close();

	sim.open((string(argv[4]) + "_pipeline.txt").c_str(), ofstream::out);
	// start sim
	int cycle = 1;

	struct fetch{
		void run(){
			if( didBreak ) return;
			for( int i = 0; i<2; i++ ){
				if( preissue[3] !=0 ) break;
				// do any checks to stop
				item I = MEM[PC];
				// check branch or jump
				// 	check for hazards

				if( I.opcode == 33 ){ // BLTZ
					if( XBW( I.src1, 4) ) break;
					if( R[I.rs] < 0 )
						PC += I.target;
					break;
				}else if( I.opcode == 34 ){ // J
					PC = I.jTarget;
					break;
				}
				// find next open spot in preissue
				// 	copy it there
				int spot = 0;
				for( ; spot < 4; spot++ ){
					if( preissue[spot] == 0 )
						break;
				}
				preissue[spot] = PC;
				PC +=4;
			}
		}
	
	};
	fetch FETCH;

	struct issue{
		void run(){
			for( int i = 0; i < 4; i++ ){
				if( preissue[i] == 0 ) continue;
				item I = MEM[preissue[i]];
				if( XBW( I.src1, i) ) continue;
				if( XBW( I.src2, i) ) continue;
				if( XBW( I.dest, i) ) continue;
				// WBR check
				// if (  LW || SW ){
				// 	if( premem[1] != 0 ) continue;
				// 	//LW SW checks
				// 	//isssue
				// 	preissue[i] = 0;
				//  }else{
				// 	// 
				// 	//issue 
				// 	preissue[i] = 0;
				//  }

			}
			for( int k =0; k < 4; k++ )
			for( int i =3; i > 0; i-- )
				if( preissue[i-1] == 0 ){
					preissue[i-1] = preissue[i];
					preissue[i] = 0;
				}
		}
	};
	issue ISSUE;

	struct memory {
		void run() {
			cout << "Running memory..." << endl;
		}
	};
	memory MEMORY;

	struct alu {
		void run() {
			cout << "Running alu..." << endl;
		}
	};
	alu ALU;

	struct wb {
		void run() {
			cout << "Running writeback..." << endl;
		}
	};
	wb WB;

	bool hasHitMemoryBreak = false;
	while( !hasHitMemoryBreak ) {
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

		runSimulation(instruction);

		PC += 4;
		cycle ++;

		WB.run();
		ALU.run();
		MEMORY.run();
		ISSUE.run();
		FETCH.run();

		if( instruction.instrStr == "BREAK" ) hasHitMemoryBreak = true;
	}
	sim.close();
}
