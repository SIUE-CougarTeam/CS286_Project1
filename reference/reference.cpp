#include <map>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <bitset>
#include <iomanip>
using namespace std;

struct item{
	int i, rs, rt, rd, imm,opcode, valid, 
		target, func, samt, jTarget, dest, src1, src2;
	string binStr, instrStr, binStrSpace;
	unsigned int asUint;
};
	int PC = 96;
	int R[32] = {0};
	int cycle = 1;
	int preissue[4] ={0};
	int premem[2] = {0};
	int prealu[2]= {0};
	int postmem=0;
	int postalu=0;
	bool didBreak = false;
	map< int, item> MEM;
	bool XBW( int rNum, int index ){
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

int main( int argc, char* argv[] )
{
	// ./mipssim -i test1.bin -o   x1
	// [0]       [1]  [2]     [3] [4]
        char buffer[4];
        int i;
	{
		int i;
	}
        char * iPtr;
        iPtr = (char*)(void*) &i;
        int FD = open( argv[2], O_RDONLY);
	//ofstream cout( string( argv[4]) + "_dis.txt");
	//ofstream simout( string( argv[4]) + "_pipeline.txt");
	int addr = 96;
	int amt = 4;
	int dataStart, dataEnd;
	while( amt != 0 )
        {
                amt = read(FD, buffer, 4);
                if( amt == 4)
                {
                        iPtr[0] = buffer[3];
                        iPtr[1] = buffer[2];
                        iPtr[2] = buffer[1];
                        iPtr[3] = buffer[0];
                	item I;
			I.i = i;
			unsigned int asUint = (unsigned int) i;
			I.asUint = asUint;
			bitset<32> b( i );
			I.binStr = b.to_string();
			I.binStrSpace =I.binStr;
			I.binStrSpace.insert( 26, " "), I.binStrSpace.insert( 21, " "), I.binStrSpace.insert( 16, " "),
				I.binStrSpace.insert( 11, " "), I.binStrSpace.insert( 6, " "), I.binStrSpace.insert( 1, " ");
			I.valid = asUint >> 31;
			I.opcode = asUint >> 26;
			I.func = asUint<<26>>26;
			I.samt = asUint<<21>>27;
			I.rs = (asUint << 6)>>27;
			I.rt = (asUint << 11)>>27;
			I.rd = (asUint << 16)>>27;
			I.imm = (i << 16) >> 16;
			I.target = I.imm << 2;
			I.jTarget = asUint <<6 >> 4;
			//cout << "valid bit: " << valid << endl;
			//cout << "opcode: " << opcode << endl;
			//cout << binstr << "\t";
			if( didBreak ){
				I.instrStr = to_string( i );
				I.binStrSpace = I.binStr;
			} else {
				if( I.valid == 0 ){
					I.instrStr = "Invalid Instruction";
				}else if( I.opcode == 40 ){
					I.instrStr ="ADDI\tR" + to_string(I.rt) + ", R"
						+ to_string(I.rs) + ", #" +to_string(I.imm);
					I.dest = I.rt;
					I.src1 = I.rs;
					I.src2 = I.rs;
				}else if( I.opcode == 43 ){
					I.instrStr ="SW\tR" + to_string(I.rt) + ", " + to_string(I.imm)
						+ "(R" + to_string(I.rs) + ")";
					I.dest = -1;
					I.src1 = I.rs;
					I.src2 = I.rt;
				}else if( I.opcode == 35 ){
					I.instrStr ="LW\tR" + to_string(I.rt) + ", " + to_string(I.imm)
						+ "(R" + to_string(I.rs) + ")";
					I.dest = I.rt;
					I.src1 = I.rs;
					I.src2 = I.rs;
				}else if( I.opcode == 33 ){
					I.instrStr ="BLTZ\tR" + to_string(I.rs) + ", #" + to_string(I.target);
					I.dest = -1;
					I.src1 = I.rs;
					I.src2 = I.rs;
				}else if( I.opcode == 34 ){
					I.instrStr ="J\t#" + to_string(I.jTarget);
				}else if( I.opcode == 32 && I.func == 0 ){
					I.instrStr ="SLL\tR" + to_string(I.rd) + ", R"
						+ to_string(I.rt) + ", #" +to_string(I.samt);
				}else if( I.opcode == 32 && I.func == 34 ){
					I.instrStr ="SUB\tR" + to_string(I.rd) + ", R"
						+ to_string(I.rs) + ", R" +to_string(I.rt);

				}else if( I.opcode == 32 && I.func == 32 ){
					I.instrStr ="ADD\tR" + to_string(I.rd) + ", R"
						+ to_string(I.rs) + ", R" +to_string(I.rt);
				}else if( I.opcode == 32 && I.func == 13 ){
					I.instrStr ="BREAK";
					didBreak = true;
					dataStart = addr+4;
				}
			}
			cout << I.binStrSpace << "\t" << addr << "\t" << I.instrStr << endl;
			MEM[addr] = I;
			addr+=4;
		}
        } // end of decode
	dataEnd = addr;
	// start sim
	didBreak = false;
	
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
				if (  LW or SW ){
					if( premem[1] != 0 ) continue;
					//LW SW checks
					//isssue
					preissue[i] = 0;
				 }else{
					// 
					//issue 
					preissue[i] = 0;
				 }

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

	while( true ){
		WB.run();
		ALU.run();
		MEMORY.run();
		ISSUE.run();
		FETCH.run();

	}
}
