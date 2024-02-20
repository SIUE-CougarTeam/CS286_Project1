
/*******************************************************
This program converts a binary file to a file 
containing string representations of the 1's and 0's
in the binfary file. 
*******************************************************/

// gcc -o b2t b2t.c
// Usage ./a.out < inputfile > outputfile

#include<stdio.h>

int main( int argc, char** argv) 
{

    if ( argc >1 )
	fprintf(stderr,"Usage: ./a.out < input_txt > output_bin\n");
    char word[32];
    unsigned char vals[4];
    unsigned char w, div, b;
    unsigned char tot ;
    
    w = 0;
    while( scanf( "%c", &tot) != EOF )
    {
	    div = 128;
	    for( b=0; b<8; b++ )
	    {
		if( tot >= div)
		{
		    printf( "1" );
		    tot -= div;
		}
		else
		    printf( "0");
		div = div/2;
	    }
	   w ++;
	   if ( w == 4 )
	   {
	       printf("\n");
	       w = 0;
	   }

    }

}
