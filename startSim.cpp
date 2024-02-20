#include <map>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <bitset>
#include <iomanip>
using namespace std;

int main()
{
        char buffer[4];
        int i;
        char * iPtr;
        iPtr = (char*)(void*) &i;

        int FD = open("test1.bin", O_RDONLY);

	struct item{
		int i, rs, rt, rd, imm,opcode, valid;
		string binStr, instrStr;
		unsigned int asUint;
	};
	map< int, item > MEM;
	int addr = 96;
        int amt = 4;
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
			instruction.binStr = b.to_string();
			instruction.valid = asUint >> 31;
			instruction.opcode = asUint >> 26;
			instruction.rs = (asUint << 6)>>27;
			instruction.rt = (asUint << 11)>>27;
			instruction.imm = (i << 16) >> 16;
			//cout << "valid bit: " << valid << endl;
			//cout << "opcode: " << opcode << endl;
			//cout << binstr << "\t"; 
			if( instruction.opcode == 40 ){
				instruction.instrStr = "ADDI\tR" + to_string(instruction.rt) + ", R"
				       + to_string(instruction.rs) + ", #" +to_string(instruction.imm);
				cout << instruction.binStr << "\t" << instruction.instrStr << endl;
			} else {
				instruction.instrStr = to_string(instruction.rt) + " " + to_string(instruction.rs) + " " + to_string(instruction.imm);
				cout << instruction.binStr << "\t" << instruction.instrStr << endl;
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
