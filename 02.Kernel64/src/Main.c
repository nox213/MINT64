#include "Types.h"

void k_print_string(int x, int y, const char *string);

void Main(void)
{
	k_print_string(0, 10, "Switch to IA-32e Mode Success");
	k_print_string(0, 11, "IA-32e C Language Kernel Start............[Pass]");
}

void k_print_string(int x, int y, const char *string)
{
	Character *pst_screen = (Character *) 0xb8000;
	int i;

	pst_screen += (y * 80) + x;
	for (i = 0; string[i] != 0; i++)
		pst_screen[i].character = string[i];
}

