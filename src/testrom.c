#include <stdio.h>
#include <stdlib.h>

#include "8080Core.h"

int main(int argc, char *argv[])
{
	reset8080(0x0100);
	
	uint8_t *bufp = registerBank(0x0000, 0x10000, NULL, 0);

	bufp[0] = 0xc3;

	// FILE *fp = fopen("CPUTEST.COM", "rb");
	FILE *fp = fopen("8080EXM.COM", "rb");
	fread(bufp + 0x100, 1, 0x10000, fp);
	fclose(fp);
	
	while (!e8080.halt)
	{		
		emulate8080(5 * 50 * 1000000);
	}
	
	return 1;
}
