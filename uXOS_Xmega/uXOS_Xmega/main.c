/*
 * uXOS_Xmega.c
 *
 * Created: 2/28/2017 2:51:25 PM
 * Author : A42739
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "uXOS/inc/uxos.h"

static UXOS_Semaphore *sem;

static uint8_t st[128];

void set_GPIO(void)
{
    PORTB.DIR |= 0xff;
    PORTB.OUTSET |= PIN0_bm;
    uxos_semaphore_pend(sem);   
}

int main(void)
{
    uxos_init();
    
    sem = uxos_semaphore_init(0);
    
    uxos_new_task( &set_GPIO, &st[127]);
    
    uxos_run();
    
    return 0;
}

