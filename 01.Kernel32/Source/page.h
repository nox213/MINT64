#ifndef __PAGE_H__
#define __PAGE_H__

#include "Types.h"

#define PAGE_FLAGS_P			0x00000001 /* present */
#define PAGE_FLAGS_RW			0x00000002 /* read, write */
#define PAGE_FLAGS_US			0x00000004 /* user or supervisor */
#define PAGE_FLAGS_PWT			0x00000008 /* page level write through */
#define PAGE_FLAGS_PCD			0x00000010 /* page level cache disable */
#define PAGE_FLAGS_A			0x00000020 /* accessed */
#define PAGE_FLAGS_D			0x00000040 /* dirty */
#define PAGE_FLAGS_PS			0x00000080 /* page size */
#define PAGE_FLAGS_G			0x00000100 /* global */
#define PAGE_FLAGS_PAT			0x00001000 /* page attribute table index */
#define PAGE_FLAGS_EXB			0x80000000 /* excute disable */
#define PAGE_FLAGS_DEFAULT		(PAGE_FLAGS_P | PAGE_FLAGS_RW)
#define PAGE_TABLE_SIZE			0x1000 /* 4KB */
#define PAGE_MAX_ENTRY_COUNT	512
#define PAGE_DEFAULT_SIZE		0x200000 /* 2MB */


#pragma pack(push, 1)

typedef struct page_table_entry_struct {
	DWORD attribute_and_lower_base_addr;
	DWORD upper_base_addr_and_exb;
} PML4ENTRY, PDPENTRY, PDENTRY, PTENTRY;

#pragma pack(pop)

void init_page_tables(void);
void set_page_entry_data(PTENTRY *entry, DWORD upper_base_addr, DWORD lower_base_addr, DWORD lower_flags, DWORD upper_flags);

#endif
