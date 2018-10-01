#include "page.h"

void init_page_tables(void)
{
	PML4ENTRY *PML4_entry;
	PDPENTRY *PDP_entry;
	PDENTRY *PD_entry;
	DWORD mapping_addr;
	int i;

	PML4_entry = (PML4ENTRY *) 0x100000;
	set_page_entry_data(&(PML4_entry[0]), 0x00, 0x101000, PAGE_FLAGS_DEFAULT, 0);
	for (i = 1; i < PAGE_MAX_ENTRY_COUNT; i++)
		set_page_entry_data(&(PML4_entry[i]), 0, 0, 0, 0);

	PDP_entry = (PDPENTRY *) 0x101000;
	for (i = 0; i < 64; i++)
		set_page_entry_data(&(PDP_entry[i]), 0, 0x102000 + (i * PAGE_TABLE_SIZE), 
							PAGE_FLAGS_DEFAULT, 0);
	for (i = 64; i < PAGE_MAX_ENTRY_COUNT; i++)
		set_page_entry_data(&(PDP_entry[i]), 0, 0, 0, 0);

	PD_entry = (PDENTRY *) 0x102000;
	mapping_addr = 0;
	for (i = 0; i < PAGE_MAX_ENTRY_COUNT * 64; i++) {
		set_page_entry_data(&(PD_entry[i]), (i * (PAGE_DEFAULT_SIZE >> 20)) >> 12, 
							mapping_addr, PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS, 0);
			mapping_addr += PAGE_DEFAULT_SIZE;
	}
}

void set_page_entry_data(PTENTRY *entry, DWORD upper_base_addr, DWORD lower_base_addr, 
						 DWORD lower_flags, DWORD upper_flags)
{
	entry->attribute_and_lower_base_addr = lower_base_addr | lower_flags;
	entry->upper_base_addr_and_exb = (upper_base_addr & 0xFF) | upper_flags;
}
