/*
 * XmegaFramework.c
 *
 * Created: 2/18/2017 10:21:36 PM
 * Author : A42739
 */ 

#include <avr/io.h>
#include <stdlib.h>

typedef struct SYSTEM_Hardware_struct {
    
    OSC_t *Oscillator;
    CLK_t * Clock;
    
}SYSTEM_Hardware;

void setHardwareInstancePointers( SYSTEM_Hardware *System )
{
    System->Oscillator = (OSC_t *)malloc(sizeof(OSC_t));
    System->Clock      = (CLK_t *)malloc(sizeof(CLK_t));
}

int main(void)
{
    SYSTEM_Hardware Sys;
    setHardwareInstancePointers( &Sys );
    
    Sys.Oscillator->XOSCCTRL |= OSC_XOSCSEL1_bm;

}

