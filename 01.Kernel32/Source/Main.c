#include "Types.h"

void k_print_string(int x, int y, const char *string);
BOOL k_initialize_kernel64_area(void);
BOOL k_is_memory_enough(void);

void Main(void)
{
	DWORD i;

	k_print_string(0, 3, "C Language Kernel Started...................[Pass]");

	k_print_string(0, 4, "Minimum Memroy Size Check...................[    ]");
	if (k_is_memory_enough() == FALSE) {
		k_print_string(45, 4, "Fail");
		k_print_string(0, 5, "Not Enough Memory~!! MINT64 OS Requires Over"
							 "64Mbyte Memory~!!");
		while(1)
			;
	}
	else 
		k_print_string(45, 4, "Pass");

	k_print_string(0, 5, "IA-32e Kernel Area Initialize...............[    ]");
	if (k_initialize_kernel64_area() == FALSE) {
		k_print_string(45, 5, "Fail");
		k_print_string(0, 6, "Kernel Area Initialization Fail~!!");
		while (1)
			;
	}
	k_print_string(45, 5, "Pass");

	while (1)
		;
}

void k_print_string(int x, int y, const char *string)
{
	Character *pst_screen = (Character *) 0xb8000;
	int i;

	pst_screen += (y * 80) + x;
	for (i = 0; string[i] != 0; i++)
		pst_screen[i].character = string[i];
}

BOOL k_initialize_kernel64_area(void)
{
	DWORD *current_address;

	current_address = (DWORD *) 0x100000;

	while ((DWORD) current_address < 0x600000) {
		*current_address = 0x00;

		if (*current_address != 0)
			return FALSE;

		current_address++;
	}

	return TRUE;
}

BOOL k_is_memory_enough(void)
{
	DWORD *current_address;

	current_address = (DWORD *) 0x100000;

	while ((DWORD) current_address < 0x4000000) {
		current_address = 0x12345678;

		/* 0x12345678로 저장 후 다시 읽어서 값이 다르면 문제가 발생한 것 */
		if (current_address != 0x12345678)
			return FALSE;

		current_address += (0x100000 / 4);
	}

	return TRUE;
}


