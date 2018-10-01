#ifndef __MODE_SWITCH_H__
#define __MODE_SWITCH_H__

#include "Types.h"

void read_cpuid(DWORD arg_eax, DWORD *eax, DWORD *ebx, DWORD *ecx, DWORD *edx);
void switch_and_execute_64bit_kernel(void);

#endif
