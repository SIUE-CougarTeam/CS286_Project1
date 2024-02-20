#include <map>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <bitset>
#include <iomanip>
#include <string>
using namespace std;

struct item {
	int i, rs, rt, rd, imm, opcode, valid;
	string binStr, instrStr;
	unsigned int asUint;
};

string formatBinaryString(string inputString) {
	string formattedString = inputString.insert(26, 1, ' ');
	formattedString = formattedString.insert(21, 1, ' '); // Subtract by 5
	formattedString = formattedString.insert(16, 1, ' ');
	formattedString = formattedString.insert(11, 1, ' ');
	formattedString = formattedString.insert(6, 1, ' ');
	formattedString = formattedString.insert(1, 1, ' ');
	return formattedString;
}

void printInstruction(item inputInstruction) {
	switch (inputInstruction.opcode) {
//		case 40:
//			break;
		default:
			inputInstruction.instrStr = "ADDI\tR" + to_string(inputInstruction.rt) + ", R"
			       + to_string(inputInstruction.rs) + ", #" + to_string(inputInstruction.imm);
			cout << inputInstruction.binStr << "\t" << inputInstruction.instrStr << endl;
			break;
	}
}

int main()
{
        char buffer[4];
        int i;
        char * iPtr;
        iPtr = (char*)(void*) &i;

        int FD = open("test1.bin", O_RDONLY);

/*	struct item{
		int i, rs, rt, rd, imm,opcode, valid;
		string binStr, instrStr;
		unsigned int asUint;
	};*/
	map< int, item > MEM;
	int addr = 96;
        int amt = 4;
	bool hasHitBreak = false;
        while( amt != 0 )
        {
                amt = read(FD, buffer, 4);
                if( amt == 4)
                {
                        iPtr[0] = buffer[3];
                        iPtr[1] = buffer[2];
                        iPtr[2] = buffer[1];
                        iPtr[3] = buffer[0];
                	item instruction;
			unsigned int asUint = (unsigned int) i;
			instruction.asUint = asUint;
			bitset<32> b( i );
			instruction.binStr = formatBinaryString(b.to_string());
			instruction.valid = asUint >> 31;
			instruction.opcode = asUint >> 26;
			instruction.rs = (asUint << 6)>>27;
			instruction.rt = (asUint << 11)>>27;
			instruction.imm = (i << 16) >> 16;
			//cout << "valid bit: " << valid << endl;
			//cout << "opcode: " << opcode << endl;
			//cout << binstr << "\t"; 
			if(!hasHitBreak){
				printInstruction(instruction);
				if (instruction.opcode == 32) { hasHitBreak = true; }
			} else {
				cout << "Memory segment." << endl;
			}
			MEM[addr] = instruction;
			addr+=4;
		}
        } // end of decode

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
}
