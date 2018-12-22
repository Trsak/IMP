/* Header file with all the essential definitions for a given type of MCU */
#include "MK60D10.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define D1 0x8C0
#define D2 0xA80
#define D3 0x2C0
#define D4 0xA40

#define DELAY_LEDD 1000

unsigned int index;

const unsigned int digitsA[] = {0x100, 0x100, 0x400, 0x500, 0x500, 0x500, 0x500, 0x100, 0x500, 0x500};
const unsigned int digitsD[] = {0xB300, 0x100, 0xB100, 0x9100, 0x300, 0x9200, 0xB200, 0x1100, 0xB300, 0x9300};

char result[10] = "0000";

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
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTD_MASK;
    SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;

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

bool detekceTepu(int analogHodnota, int zpozdeni) {
  // vytvo�en� pomocn�ch prom�nn�ch
  static int maxHodnota = 0;
  static bool SpickovaHodnota = false;
  bool vysledek = false;
  // p�epo�et analogov� hodnoty pro dal�� v�po�ty
  analogHodnota *= (1000 / zpozdeni);
  // upraven� maxim�ln� hodnoty
  if (analogHodnota * 4L < maxHodnota) {
    maxHodnota = analogHodnota * 0.8;
  }
  // detekce �pi�kov� hodnoty
  if (analogHodnota > maxHodnota - (1000 / zpozdeni)) {
    // nastaven� nov�ho maxima p�i detekovan� �pi�ce
    if (analogHodnota > maxHodnota) {
      maxHodnota = analogHodnota;
    }
    // nastaven� platnosti v�sledku, kdy�
    // nebyla detekov�na �pi�ka
    if (SpickovaHodnota == false) {
      vysledek = true;
    }
    SpickovaHodnota = true;
  } else if (analogHodnota < maxHodnota - (3000 / zpozdeni)) {
    SpickovaHodnota = false;
    // upraven� maxim�ln� hodnoty p�i zm�n� m��en�ch hodnot
    maxHodnota -= (1000 / zpozdeni);
  }
  // vr�cen� v�sledku podprogramu
  return vysledek;
}

void ADC0_Init(void)
{
	NVIC_ClearPendingIRQ(ADC0_IRQn);
    NVIC_EnableIRQ(ADC0_IRQn);

    ADC0_CFG1 = 0b01110101;
    ADC0_SC3 = 0b00001111;
    ADC0_SC1A = 0b01010000;
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

const int zpozdeniMereni = 60 + 4;
int frekvence = 0;

void ADC0_IRQHandler(void) {
	int value = ADC0_RA;

	static int uderyZaMinutu = 0;
	int tepovaFrekvence = 0;

	if (detekceTepu(value, zpozdeniMereni)) {
		frekvence = 60000 / uderyZaMinutu;
		uderyZaMinutu = 0;
	}

	uderyZaMinutu += 60;
}

int main(void)
{
    MCUInit();
    PortsInit();
    ADC0_Init();

    while (1) {
    	static int delayedResult = 0;
    	index = 0;

    	if (delayedResult >= 100) {
			sprintf(result, "%04d", frekvence);
			delayedResult = 0;
    	}

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

    	++delayedResult;
    }

    return 0;
}
