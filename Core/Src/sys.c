#include "sys.h"

void WFI_SET(void)
{
    __asm("WFI");
}
void INTX_DISABLE(void)
{
    __asm("CPSID I");
    __asm("BX LR");
}

void INTX_ENABLE(void)
{
    __asm("CPSIE I");
    __asm("BX LR");
}
void MSR_MSP(u32 addr)
{
    __asm("MSR MSP, r0");
    __asm("BX r14");
}
