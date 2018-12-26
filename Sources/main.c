//Made by Petr Sopf (xsopfp00)

/* Header file with all the essential definitions for a given type of MCU */
#include "MK60D10.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

//Display segments
#define D1 0x8C0
#define D2 0xA80
#define D3 0x2C0
#define D4 0xA40

//Display LEDs delay
#define DELAY_LEDD 1000

//Index variable used for displaying string on display
unsigned int index;

//Display digits
const unsigned int digitsA[] = {0x100, 0x100, 0x400, 0x500, 0x500, 0x500, 0x500, 0x100, 0x500, 0x500};
const unsigned int digitsD[] = {0xB300, 0x100, 0xB100, 0x9100, 0x300, 0x9200, 0xB200, 0x1100, 0xB300, 0x9300};

//String displayed on display
char stringBPM[10] = "0000";

//Current BPM value
int heartRateBPM = 0;

//Current analog signal value
int analogValue;

//Delay function
void delay(long long bound) {
  long long i;
  for(i=0;i<bound;i++);
}

//Detect heartbeat in analog signal
//Taken from module documentation: https://www.gme.cz/data/attachments/dsh.775-025.1.pdf
bool heartbeatDetected(int rawValue, int delay)
{
	static int maxValue = 0;
	static bool isPeak = false;
	bool stringBPM = false;

	rawValue *= (1000/delay);

	if (rawValue * 4L < maxValue) {
		maxValue = rawValue * 0.8;
	}

	if (rawValue > maxValue - (1000/delay)) {
		if (rawValue > maxValue) {
			maxValue = rawValue;
		}

		if (isPeak == false) {
			stringBPM = true;
		}

		isPeak = true;
	} else if (rawValue < maxValue - (3000/delay)) {
		isPeak = false;
		maxValue-=(1000/delay);
	}

	return stringBPM;
}

//Initialize the MCU - basic clock settings, turning the watchdog off
void MCUInit(void)  {
    MCG_C4 |= ( MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(0x01) );
    SIM_CLKDIV1 |= SIM_CLKDIV1_OUTDIV1(0x00);
    WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;
}

//Initialize ports
void PortsInit(void)
{
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_LPTIMER_MASK;
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

//Initialize ADC0 to send interrupts
void ADC0_Init(void)
{
	NVIC_ClearPendingIRQ(ADC0_IRQn);
    NVIC_EnableIRQ(ADC0_IRQn);

    ADC0_CFG1 = 0b01110101;
    ADC0_SC3 = 0b00001111;
    ADC0_SC1A = 0b01010000;
}

//Initialize LPTMR0 to send interrupts every 60ms
void LPTMR0_Init(void)
{
	LPTMR0_PSR = LPTMR_PSR_PRESCALE(0) | LPTMR_PSR_PBYP_MASK | LPTMR_PSR_PCS(1);
	LPTMR0_CSR = LPTMR_CSR_TCF_MASK | LPTMR_CSR_TIE_MASK;
	LPTMR0_CMR = LPTMR_CMR_COMPARE(60);

	NVIC_ClearPendingIRQ(LPTMR0_IRQn);
    NVIC_EnableIRQ(LPTMR0_IRQn);
    LPTMR0_CSR |= LPTMR_CSR_TEN_MASK;
}

//Handling ADC0 interrupts
void ADC0_IRQHandler(void) {
	analogValue = ADC0_RA;
}

//Handling LPTMR0 interrupts
void LPTMR0_IRQHandler(void) {
	static int beatMsec = 0;
	int delayMsec = 60;

	LPTMR0_CSR &= ~LPTMR_CSR_TEN_MASK;

	//Check if heartbeat is detected
	if (heartbeatDetected(analogValue, delayMsec)) {
		//Heartbeat is detected, calculate BPM
		int tempHeartRateBPM = 60000 / beatMsec;

		if (tempHeartRateBPM > 40 & tempHeartRateBPM < 200) {
			heartRateBPM = tempHeartRateBPM;
		}

		beatMsec = 0;
	}

	beatMsec += delayMsec;

	//Next interrupt in 60ms
    LPTMR0_CSR |= LPTMR_CSR_TEN_MASK;
}

//For displaying string value at display
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

int main(void)
{
	//Initialize
    MCUInit();
    PortsInit();
    ADC0_Init();
    LPTMR0_Init();

    while (1) {
    	//Delay showing new BPM value
    	static int delayedResult = 0;

    	index = 0;

    	if (delayedResult >= 100) {
			sprintf(stringBPM, "%04d", heartRateBPM);
			delayedResult = 0;
    	}

    	display_val(stringBPM);
    	PTA->PDOR |= GPIO_PDOR_PDO(D1);
    	delay(DELAY_LEDD);
    	PTA->PDOR &= GPIO_PDOR_PDO(~D1);

    	display_val(stringBPM);
    	PTA->PDOR |= GPIO_PDOR_PDO(D2);
    	delay(DELAY_LEDD);
    	PTA->PDOR &= GPIO_PDOR_PDO(~D2);

    	display_val(stringBPM);
    	PTA->PDOR |= GPIO_PDOR_PDO(D3);
    	delay(DELAY_LEDD);
    	PTA->PDOR &= GPIO_PDOR_PDO(~D3);

    	display_val(stringBPM);
    	PTA->PDOR |= GPIO_PDOR_PDO(D4);
    	delay(DELAY_LEDD);
    	PTA->PDOR &= GPIO_PDOR_PDO(~D4);

    	++delayedResult;
    }

    return 0;
}
