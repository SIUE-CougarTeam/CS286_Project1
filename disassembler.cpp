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

        int amt = 4;
        while( amt != 0 )
        {
                amt = read(FD, buffer, 4);
                if( amt == 4 )
                {
                        iPtr[0] = buffer[3];
                        iPtr[1] = buffer[2];
                        iPtr[2] = buffer[1];
                        iPtr[3] = buffer[0];
                        cout << "i = " << hex << i << endl;

			unsigned int asUint = (unsigned int) i;
			bitset<32> b( i );
			string binstr = b.to_string();
			int valid = asUint >> 31;
			int opcode = asUint >> 26;
			int rs = (asUint << 6) >> 27;
			cout << "valid bit: " << valid << endl;
                        cout << "op code: " << opcode << endl;
                        cout << "rs bits: " << rs << endl;
			cout << binstr << "\t";
			if ( opcode == 40 ){
				cout << "ADDI\tR , R" << rs << endl;
			}
                }
        }







}
