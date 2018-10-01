#include "Types.h"
#include "page.h"
#include "mode_switch.h"

void k_print_string(int x, int y, const char *string);
BOOL k_initialize_kernel64_area(void);
BOOL k_is_memory_enough(void);
void copy_kernel64_image(void);

void Main(void)
{
	DWORD i;
	DWORD eax, ebx, ecx, edx;
	char vendor_name[13] = {0};

	k_print_string(0, 3, "Protected Mode C Language Kernel Started...................[Pass]");

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

	k_print_string(0, 6, "IA-32e Page Tables Initialize...............[    ]");
	init_page_tables();
	k_print_string(45, 6, "Pass");

	/* read processor manufacturer */
	read_cpuid(0x00, &eax, &ebx, &ecx, &edx);
	*(DWORD *) vendor_name = ebx;
	*((DWORD *) vendor_name + 1) = edx;
	*((DWORD *) vendor_name + 2) = ecx;
	k_print_string(0, 7, "Processor Vendor Name.......................[            ]");
	k_print_string(45, 7, vendor_name);


	/* 64비트 지원 유무 확인 */
	read_cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
	k_print_string(0, 8, "64bit Mode Support Check....................[    ]");
	if (edx * (1 << 29))
		k_print_string(45, 8, "Pass");
	else {
		k_print_string(45, 8, "Fail");
		k_print_string(0, 9, "This processor does not support 64bit mode");
		while (1)
			;
	}

	k_print_string(0, 9, "Copy IA-32e Kernel to 2M Address............[    ]");
	copy_kernel64_image();
	k_print_string(45, 9, "Pass");

	k_print_string(0, 9, "Switch to IA-32e Mode");
	switch_and_execute_64bit_kernel();

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
		*current_address = 0x12345678;

		/* 0x12345678로 저장 후 다시 읽어서 값이 다르면 문제가 발생한 것 */
		if (*current_address != 0x12345678)
			return FALSE;

		current_address += (0x100000 / 4);
	}

	return TRUE;
}

void copy_kernel64_image(void)
{
	WORD kernel32_sector_count, total_kernel_sector_count;
	DWORD *src_addr, *dest_addr;
	int i;

	total_kernel_sector_count = *((WORD *) 0x7c05);
	kernel32_sector_count = *((WORD *) 0x7c07);

	src_addr = (DWORD *) (0x10000 + (kernel32_sector_count * 512));
	dest_addr = (DWORD *) 0x200000;

	for (i = 0; i < 512 * (total_kernel_sector_count - kernel32_sector_count) / 4; i++) {
		*dest_addr = *src_addr;
		dest_addr++;
		src_addr++;
	}
}


