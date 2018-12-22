/* Header file with all the essential definitions for a given type of MCU */
#include "MK60D10.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define D1 0x8C0
#define D2 0xA80
#define D3 0x2C0
#define D4 0xA40

#define DELAY_LEDD 1000

unsigned int index;

const unsigned int digitsA[] = {0x100, 0x100, 0x400, 0x500, 0x500, 0x500, 0x500, 0x100, 0x500, 0x500};
const unsigned int digitsD[] = {0xB300, 0x100, 0xB100, 0x9100, 0x300, 0x9200, 0xA200, 0x1100, 0xB300, 0x9300};

/* A delay function */
void delay(long long bound) {
  long long i;
  for(i=0;i<bound;i++);
}

/* Initialize the MCU - basic clock settings, turning the watchdog off */
void MCUInit(void)  {
    MCG_C4 |= ( MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(0x01) );
    SIM_CLKDIV1 |= SIM_CLKDIV1_OUTDIV1(0x00);
    WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;
}

void PortsInit(void)
{
    SIM->SCGC5 = SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTD_MASK;

    PORTA->PCR[6] = PORT_PCR_MUX(0x01);
    PORTA->PCR[7] = PORT_PCR_MUX(0x01);
    PORTA->PCR[8] = PORT_PCR_MUX(0x01);
    PORTA->PCR[9] = PORT_PCR_MUX(0x01);
    PORTA->PCR[10] = PORT_PCR_MUX(0x01);
    PORTA->PCR[11] = PORT_PCR_MUX(0x01);

    PORTD->PCR[8] = PORT_PCR_MUX(0x01);
    PORTD->PCR[9] = PORT_PCR_MUX(0x01);
    PORTD->PCR[12] = PORT_PCR_MUX(0x01);
    PORTD->PCR[13] = PORT_PCR_MUX(0x01);
    PORTD->PCR[14] = PORT_PCR_MUX(0x01);
    PORTD->PCR[15] = PORT_PCR_MUX(0x01);

    PTA->PDDR |= GPIO_PDDR_PDD(0xFC0);
    PTD->PDDR |= GPIO_PDDR_PDD(0xF300);
}

void display_val(char *val_str) {
    if (index >= strlen(val_str) || index > 4)
    {
        return;
    }

    if (isdigit(val_str[index]))
    {
        PTA->PDOR = GPIO_PDOR_PDO(digitsA[val_str[index]-'0']);
        PTD->PDOR = GPIO_PDOR_PDO(digitsD[val_str[index]-'0']);
        index++;
    }
    else
    {
        index++;
    }
}

char result[10] = "0105";

int main(void)
{
    MCUInit();
    PortsInit();

    /*

      D1                    D2                    D3                    D4
    		A
    	---------             ---------             ---------             ---------
       |	     |           |         |           |         |           |         |
       |         |           |         |           |         |           |         |
     F |	     | B         |         |           |         |           |         |
       |         |           |         |           |         |           |         |
       |    G    |           |         |           |         |           |         |
    	---------             ---------             ---------             ---------
       |         |           |         |           |         |           |         |
       |         |           |         |           |         |           |         |
     E |         | C         |         |           |         |           |         |
       |         |           |         |           |         |           |         |
       |         |           |         |           |         |           |         |
    	---------   o         ---------   o         ---------   o         ---------   o
    		D       DP

    A = 0x1000 PTD
    B = 0x100 PTD
    D = 0x8000 PTD
    E = 0x2000 PTD
    F = 0x200 PTD

    C = 0x100 PTA
    G = 0x400 PTA

	D1 = 0x0200 PTA
	D2 = 0x0040 PTA
	D3 = 0x0800 PTA
	D4 = 0x0080 PTA

    */

    while (1) {
    	index = 0;

    	display_val(result);
    	PTA->PDOR |= GPIO_PDOR_PDO(D1);
    	delay(DELAY_LEDD);
    	PTA->PDOR &= GPIO_PDOR_PDO(~D1);

    	display_val(result);
    	PTA->PDOR |= GPIO_PDOR_PDO(D2);
    	delay(DELAY_LEDD);
    	PTA->PDOR &= GPIO_PDOR_PDO(~D2);

    	display_val(result);
    	PTA->PDOR |= GPIO_PDOR_PDO(D3);
    	delay(DELAY_LEDD);
    	PTA->PDOR &= GPIO_PDOR_PDO(~D3);

    	display_val(result);
    	PTA->PDOR |= GPIO_PDOR_PDO(D4);
    	delay(DELAY_LEDD);
    	PTA->PDOR &= GPIO_PDOR_PDO(~D4);
    }

    return 0;
}
