﻿// Vzorovя┐╜ projekt pro STM8S103F 
#include "stm8s.h"
#include "delay.h"
#include "milis.h"
#include "spse_stm8.h"
//#include "stm8_hd44780.h


// makra kterя┐╜mi volя┐╜me komunikacnя┐╜ piny
#define CLK_GPIO GPIOC				// port na kterя┐╜m je CLK vstup budice
#define CLK_PIN  GPIO_PIN_5		// pin na kterя┐╜m je CLK vstup budice
#define DATA_GPIO GPIOC				// port na kterя┐╜m je DIN vstup budice
#define DATA_PIN  GPIO_PIN_7	// pin na kterя┐╜m je DIN vstup budice
#define CS_GPIO GPIOC					// port na kterя┐╜m je LOAD/CS vstup budice
#define CS_PIN  GPIO_PIN_6		// pin na kterя┐╜m je LOAD/CS vstup budice

// makra kterя┐╜ zprehlednujя┐╜ zdrojovя┐╜ kя┐╜d a delajя┐╜ ho snя┐╜ze prenositelnя┐╜m na jinя┐╜ mikrokontrolя┐╜ry a platformy
#define CLK_HIGH 			GPIO_WriteHigh(CLK_GPIO, CLK_PIN)
#define CLK_LOW 			GPIO_WriteLow(CLK_GPIO, CLK_PIN)
#define DATA_HIGH 		GPIO_WriteHigh(DATA_GPIO, DATA_PIN)
#define DATA_LOW 			GPIO_WriteLow(DATA_GPIO, DATA_PIN)
#define CS_HIGH 			GPIO_WriteHigh(CS_GPIO, CS_PIN)
#define CS_LOW 				GPIO_WriteLow(CS_GPIO, CS_PIN)

// makra adres/prя┐╜kazu pro citelnejя┐╜я┐╜ ovlя┐╜dя┐╜nя┐╜ MAX7219
#define NOOP 					0  	// No operation
#define DIGIT0 				1		// zя┐╜pis hodnoty na 1. cifru
#define DIGIT1 				2		// zя┐╜pis hodnoty na 1. cifru
#define DIGIT2 				3		// zя┐╜pis hodnoty na 1. cifru
#define DIGIT3 				4		// zя┐╜pis hodnoty na 1. cifru
#define DIGIT4 				5		// zя┐╜pis hodnoty na 1. cifru
#define DIGIT5 				6		// zя┐╜pis hodnoty na 1. cifru
#define DIGIT6 				7		// zя┐╜pis hodnoty na 1. cifru
#define DIGIT7 				8		// zя┐╜pis hodnoty na 1. cifru
#define DECODE_MODE 	9		// Aktivace/Deaktivace znakovя┐╜ sady (my volя┐╜me vя┐╜dy hodnotu DECODE_ALL)
#define INTENSITY 		10	// Nastavenя┐╜ jasu - argument je cя┐╜slo 0 aя┐╜ 15 (vetя┐╜я┐╜ cя┐╜slo vetя┐╜я┐╜ jas)
#define SCAN_LIMIT 		11	// Volba poctu cifer (velikosti displeje) - argument je cя┐╜slo 0 aя┐╜ 7 (my dя┐╜vя┐╜me vя┐╜dy 7)
#define SHUTDOWN 			12	// Aktivace/Deaktivace displeje (ON / OFF)
#define DISPLAY_TEST 	15	// Aktivace/Deaktivace "testu" (rozsvя┐╜tя┐╜ vя┐╜echny segmenty)

// makra argumentu
// argumenty pro SHUTDOWN
#define DISPLAY_ON		1		// zapne displej
#define DISPLAY_OFF		0		// vypne displej
// argumenty pro DISPLAY_TEST
#define DISPLAY_TEST_ON 	1	// zapne test displeje
#define DISPLAY_TEST_OFF 	0	// vypne test displeje
// argumenty pro DECODE_MOD
#define DECODE_ALL			0b11111111 // (lepя┐╜я┐╜ zя┐╜pis 0xff) zapя┐╜nя┐╜ znakovou sadu pro vя┐╜echny cifry
#define DECODE_NONE			0 // vypя┐╜nя┐╜ znakovou sadu pro vя┐╜echny cifry// odeя┐╜le do budice MAX7219 16bitovя┐╜ cя┐╜slo sloя┐╜enя┐╜ z prvnя┐╜ho a druhя┐╜ho argumentu (nejprve adresa, potя┐╜ data)

// ovladani nabijeni/vybijeni
#define discharge_port GPIOD
#define discharge_pin GPIO_PIN_3

#define discharge_chip_select_port GPIOD
#define discharge_chip_select_pin GPIO_PIN_6

#define discharge_increment_port GPIOD
#define discharge_increment_pin GPIO_PIN_5

#define discharge_up_down_port GPIOD
#define discharge_up_down_pin GPIO_PIN_4

#define charge_port GPIOD
#define charge_pin GPIO_PIN_1

#define charge_chip_select_port GPIOA
#define charge_chip_select_pin GPIO_PIN_1

#define charge_increment_port GPIOA
#define charge_increment_pin GPIO_PIN_2

#define charge_up_down_port GPIOA
#define charge_up_down_pin GPIO_PIN_3

uint16_t charge_counter = 0;
uint16_t discharge_counter = 0;
uint16_t d[3] = {2, 5, 20}; // nastaveni vybijeciho proudu 2=100mA, 5=253mA, 20=1009mA
uint16_t ch = 80; //nastaveni nabijeciho proudu =100mA, =250mA, =1000mA
uint8_t second_charge_cycle = 0;

void max7219_posli(uint8_t adresa, uint8_t data){ // posle zpravu displayi, z elektromys 
	uint8_t maska; // pomocnя┐╜ promennя┐╜, kterя┐╜ bude slouя┐╜it k prochя┐╜zenя┐╜ dat bit po bitu
	CS_LOW; // nastavя┐╜me linku LOAD/CS do я┐╜rovne Low (abychom po zapsя┐╜nя┐╜ vя┐╜ech 16ti bytu mohli vygenerovat na CS vzestupnou hranu)

	// nejprve odeя┐╜leme prvnя┐╜ch 8bitu zprя┐╜vy (adresa/prя┐╜kaz)
	maska = 0b10000000; // lepя┐╜я┐╜ zя┐╜pis je: maska = 1<<7
	CLK_LOW; // pripravя┐╜me si na CLK vstup budice я┐╜roven Low
	while(maska){ // dokud jsme neposlali vя┐╜ech 8 bitu
		if(maska & adresa){ // pokud mя┐╜ prя┐╜ve vysя┐╜lanя┐╜ bit hodnotu 1
			DATA_HIGH; // nastavя┐╜me budici vstup DIN do я┐╜rovne High
		}
		else{ // jinak mя┐╜ prя┐╜ve vysя┐╜lanя┐╜ bit hodnotu 0 a...
			DATA_LOW;	// ... nastavя┐╜me budici vstup DIN do я┐╜rovne Low
		}
		CLK_HIGH; // prejdeme na CLK z я┐╜rovne Low do я┐╜rovne High, a budic si zapя┐╜e hodnotu bitu, kterou jsme nastavili na DIN
		maska = maska>>1; // rotujeme masku abychom v prя┐╜tя┐╜m kroku vysя┐╜lali niя┐╜я┐╜я┐╜ bit
		CLK_LOW; // vrя┐╜tя┐╜me CLK zpet do Low abychom mohli celя┐╜ proces vysя┐╜lя┐╜nя┐╜ bitu opakovat
	}

	// potя┐╜ poя┐╜leme dolnя┐╜ch 8 bitu zprя┐╜vy (data/argument)
	maska = 0b10000000;
	while(maska){ // dokud jsme neposlali vя┐╜ech 8 bitu
		if(maska & data){ // pokud mя┐╜ prя┐╜ve vysя┐╜lanя┐╜ bit hodnotu 1
			DATA_HIGH; // nastavя┐╜me budici vstup DIN do я┐╜rovne High
		}
		else{ // jinak mя┐╜ prя┐╜ve vysя┐╜lanя┐╜ bit hodnotu 0 a...
			DATA_LOW;	// ... nastavя┐╜me budici vstup DIN do я┐╜rovne Low
		}
		CLK_HIGH; // prejdeme na CLK z я┐╜rovne Low do я┐╜rovne High, a v budic si zapя┐╜e hodnotu bitu, kterou jsme nastavili na DIN
		maska = maska>>1; // rotujeme masku abychom v prя┐╜tя┐╜m kroku vysя┐╜lali niя┐╜я┐╜я┐╜ bit
		CLK_LOW; // vrя┐╜tя┐╜me CLK zpet do Low abychom mohli celя┐╜ proces vysя┐╜lя┐╜nя┐╜ bitu opakovat
	}

	CS_HIGH; // nastavя┐╜me LOAD/CS z я┐╜rovne Low do я┐╜rovne High a vygenerujeme tя┐╜m vzestupnou hranu (pokyn pro MAX7219 aby zpracoval nя┐╜ prя┐╜kaz)
}

// nastavя┐╜ CLK,LOAD/CS,DATA jako vя┐╜stupy a nakonfiguruje displej
void max7219_init(void){
GPIO_Init(CS_GPIO,CS_PIN,GPIO_MODE_OUT_PP_LOW_SLOW);
GPIO_Init(CLK_GPIO,CLK_PIN,GPIO_MODE_OUT_PP_LOW_SLOW);
GPIO_Init(DATA_GPIO,DATA_PIN,GPIO_MODE_OUT_PP_LOW_SLOW);
// nastavя┐╜me zя┐╜kladnя┐╜ parametry budice
max7219_posli(DECODE_MODE, 0b11111100); // zapnout znakovou sadu na vя┐╜ech cifrя┐╜ch krome poslednich dvou 
max7219_posli(SCAN_LIMIT, 7); // velikost displeje 8 cifer (pocя┐╜tя┐╜no od nuly, proto je argument cя┐╜slo 7)
max7219_posli(INTENSITY, 5); // volя┐╜me ze zacя┐╜tku nя┐╜zkя┐╜ jas (vysokя┐╜ jas muя┐╜e mя┐╜t velkou spotrebu - aя┐╜ 0.25A !)
max7219_posli(DISPLAY_TEST, DISPLAY_TEST_OFF); // 
max7219_posli(SHUTDOWN, DISPLAY_ON); // zapneme displej
}

// pro milis -> poslednя┐╜ cas provedeni funkce
static uint16_t last_time_timer=0;
static uint16_t last_time_charge=0;

// mereny cas v sekundach, 16 bitove neznamenkove 
// -> rozsah 0-2^16 -> neco pres 18 hodin
uint16_t time_in_s = 0;

// promene pro zapis prubezneho casu
// ve formatu hodiny, minuty, sekundy
uint8_t hours, minutes, seconds;

// timer_on -> 0 timer je vypnuty, nepocita cas
// timer_on -> 1 timer je zapnutym pocita cas
uint8_t timer_on = 0; 

// timer_reset -> 0 nic se nedeje
// timer_reset -> 1 kdykoli nastane, pri dalsim pocitani casu
// 								se cas resetuje na 0s 
uint8_t timer_reset = 0;

// for setting discharge value
uint8_t discharge_setting = 0;

// for indication if charge is on
uint8_t charge_is_on = 0;

// charging animation
uint8_t charge_animation = 0;

// decimal point ticking
uint8_t decimal_point = 0;

void charge_discharge_init(void)
{   //discharge
	GPIO_Init(discharge_chip_select_port, discharge_chip_select_pin, GPIO_MODE_OUT_PP_LOW_SLOW);
	GPIO_Init(discharge_increment_port, discharge_increment_pin, GPIO_MODE_OUT_PP_HIGH_SLOW);
	GPIO_Init(discharge_up_down_port, discharge_up_down_pin, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(discharge_port, discharge_pin, GPIO_MODE_OUT_PP_HIGH_SLOW);
    //charge
    GPIO_Init(charge_chip_select_port, charge_chip_select_pin, GPIO_MODE_OUT_PP_LOW_SLOW);
	GPIO_Init(charge_increment_port, charge_increment_pin, GPIO_MODE_OUT_PP_HIGH_SLOW);
	GPIO_Init(charge_up_down_port, charge_up_down_pin, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(charge_port, charge_pin, GPIO_MODE_OUT_PP_HIGH_SLOW);
}

void discharge_up_resistance(void)
{
	GPIO_WriteHigh(discharge_up_down_port, discharge_up_down_pin);
	GPIO_WriteLow(discharge_increment_port, discharge_increment_pin);
	_delay_us(100);
	GPIO_WriteHigh(discharge_increment_port, discharge_increment_pin);
}

void discharge_down_resistance(void)
{
	GPIO_WriteLow(discharge_up_down_port, discharge_up_down_pin);
	GPIO_WriteLow(discharge_increment_port, discharge_increment_pin);
	_delay_us(100);
	GPIO_WriteHigh(discharge_increment_port, discharge_increment_pin);
}

void charge_up_resistance(void)
{
	GPIO_WriteHigh(charge_up_down_port, charge_up_down_pin);
	GPIO_WriteLow(charge_increment_port, charge_increment_pin);
	_delay_us(100);
	GPIO_WriteHigh(charge_increment_port, charge_increment_pin);
} //(5000mah 3,5V baterie pro nabijeni)

void charge_down_resistance(void)
{
	GPIO_WriteLow(charge_up_down_port, charge_up_down_pin);
	GPIO_WriteLow(charge_increment_port, charge_increment_pin);
	_delay_us(100);
	GPIO_WriteHigh(charge_increment_port, charge_increment_pin);
}

void discharge_setup(void)
{
    while (discharge_counter < 100)
    {
        discharge_up_resistance();
        discharge_counter++;
    } 
    discharge_counter = 0;     
    while (discharge_counter < d[discharge_setting])
    {
        discharge_down_resistance();
        discharge_counter++;
    } 
}

void charge_setup(void)
{
	while (charge_counter < 100)
    {
        charge_down_resistance();
        charge_counter++;
    }
    charge_counter = 0;
    while (charge_counter < ch)
    {
        charge_up_resistance();
        charge_counter++;
    } 
}

void discharge_on(void)
{
	GPIO_WriteLow(discharge_port, discharge_pin);
}

void discharge_off(void)
{
	GPIO_WriteHigh(discharge_port, discharge_pin);
}

void charge_on(void)
{
	GPIO_WriteHigh(charge_port, charge_pin);
}

void charge_off(void)
{
	GPIO_WriteLow(charge_port, charge_pin);
}

void show_discharge_current(uint8_t mode){
	max7219_posli(DIGIT1, 0b01110111);
	if (mode==1){
		max7219_posli(DECODE_MODE, 0b11111100);
		max7219_posli(DIGIT0, 0);
		max7219_posli(DIGIT1, 0b01110111);
		max7219_posli(DIGIT2, 5);
		max7219_posli(DIGIT3, 2);
		max7219_posli(DIGIT4, 0b10000000);
		
		// blank
		max7219_posli(DIGIT5, 0b1111);
		max7219_posli(DIGIT6, 0b1111);
		max7219_posli(DIGIT7, 0b1111);
	}
	else if (mode==0){
		max7219_posli(DECODE_MODE, 0b11111100);
		max7219_posli(DIGIT1, 0b01110111);
		max7219_posli(DIGIT2, 1);
		max7219_posli(DIGIT3, 0b10000000);
		
		// blank
		max7219_posli(DIGIT0, 0);
		max7219_posli(DIGIT4, 0b1111);
		max7219_posli(DIGIT5, 0b1111);
		max7219_posli(DIGIT6, 0b1111);
		max7219_posli(DIGIT7, 0b1111);
	}
	
	else if (mode==2){
		max7219_posli(DECODE_MODE, 0b11111100);
		max7219_posli(DIGIT1, 0b01110111);
		max7219_posli(DIGIT2, 1);
		
		// blank
		max7219_posli(DIGIT0, 0);
		max7219_posli(DIGIT3, 0b1111);
		max7219_posli(DIGIT4, 0b1111);
		max7219_posli(DIGIT5, 0b1111);
		max7219_posli(DIGIT6, 0b1111);
		max7219_posli(DIGIT7, 0b1111);
	}
}

void show_Ah(void){
	max7219_posli(DECODE_MODE, 0b11111100);
	max7219_posli(DIGIT0, 0b00010111);
	max7219_posli(DIGIT1, 0b01110111);
}

void show_voltage(void){
	max7219_posli(DIGIT0, 0b00000000);
	max7219_posli(DIGIT1, 0b00111110);
	max7219_posli(DIGIT2, 9);
	max7219_posli(DIGIT3, 2);
	max7219_posli(DIGIT4, 0b10000100);
	
	max7219_posli(DIGIT5, 0b1111);
	max7219_posli(DIGIT6, 0b1111);
	max7219_posli(DIGIT7, 0b1111);
}

void charge_animate(uint8_t charge_animation){
	if (charge_animation == 0){
		max7219_posli(DECODE_MODE, 0b11111100);
		max7219_posli(DIGIT0, 0b00000001);
		max7219_posli(DIGIT1, 0b00000001);
		
		max7219_posli(DIGIT2, 0b00001111);
		max7219_posli(DIGIT3, 0b00001111);
		
		max7219_posli(DIGIT4, 0b00001111);
		max7219_posli(DIGIT5, 0b00001111);
		
		max7219_posli(DIGIT6, 0b00001111);
		max7219_posli(DIGIT7, 0b00001111);
	}
	
	else if (charge_animation == 1){
		max7219_posli(DECODE_MODE, 0b11111100);
		max7219_posli(DIGIT0, 0b00000000);
		max7219_posli(DIGIT1, 0b00000000);
		
		max7219_posli(DIGIT2, 0b00001010);
		max7219_posli(DIGIT3, 0b00001010);
		
		max7219_posli(DIGIT4, 0b00001111);
		max7219_posli(DIGIT5, 0b00001111);
		
		max7219_posli(DIGIT6, 0b00001111);
		max7219_posli(DIGIT7, 0b00001111);
	}
	
	else if (charge_animation == 2){
		max7219_posli(DECODE_MODE, 0b11111100);
		max7219_posli(DIGIT0, 0b00000000);
		max7219_posli(DIGIT1, 0b00000000);
		
		max7219_posli(DIGIT2, 0b00001111);
		max7219_posli(DIGIT3, 0b00001111);
		
		max7219_posli(DIGIT4, 0b00001010);
		max7219_posli(DIGIT5, 0b00001010);
		
		max7219_posli(DIGIT6, 0b00001111);
		max7219_posli(DIGIT7, 0b00001111);
	}
	
	else if (charge_animation == 3){
		max7219_posli(DECODE_MODE, 0b11111100);
		max7219_posli(DIGIT0, 0b00000000);
		max7219_posli(DIGIT1, 0b00000000);
		
		max7219_posli(DIGIT2, 0b00001111);
		max7219_posli(DIGIT3, 0b00001111);
		
		max7219_posli(DIGIT4, 0b00001111);
		max7219_posli(DIGIT5, 0b00001111);
		
		max7219_posli(DIGIT6, 0b00001010);
		max7219_posli(DIGIT7, 0b00001010);
	}
}

void display_hours(uint8_t hours){
	max7219_posli(DIGIT7, hours/10);
	max7219_posli(DIGIT6, hours%10+decimal_point);
}

void display_minutes(uint8_t minutes){
	max7219_posli(DIGIT5, minutes/10);
	max7219_posli(DIGIT4, minutes%10+decimal_point);
}

void display_seconds(uint8_t seconds){
	max7219_posli(DIGIT3, seconds/10);
	max7219_posli(DIGIT2, seconds%10+decimal_point);
}

void show_capacity(uint16_t time_in_s, uint8_t discharge_setting){
	uint32_t capacity = 0;
	uint8_t tisicina, setina, desetina, jednotka;
	
	if (discharge_setting==0){
		capacity = 100 * minutes;
	}
	else if (discharge_setting==1){
		capacity = 250 * minutes;
	} 
	else if (discharge_setting==2){
		capacity = 1000 * minutes;
	}
	
	capacity /= 60;
	
	jednotka = (capacity / 1000)% 10;
	tisicina = capacity % 10;
	setina = (capacity / 10) % 10;
	desetina = (capacity / 100) % 10;
	
	max7219_posli(DECODE_MODE, 0b11111100);
	max7219_posli(DIGIT2, jednotka);
	max7219_posli(DIGIT3, setina);
	max7219_posli(DIGIT4, desetina);
	max7219_posli(DIGIT5, jednotka+128);
	max7219_posli(DIGIT6, 0b1111);
	max7219_posli(DIGIT7, 0b1111);
	show_Ah();
}

void setup_ADCs(void){
	// GPIOC, PIN4
	ADC1_SchmittTriggerConfig(ADC1_SCHMITTTRIG_CHANNEL2,DISABLE);
	// nastavя┐╜me clock pro ADC (16MHz / 4 = 4MHz)
	ADC1_PrescalerConfig(ADC1_PRESSEL_FCPU_D4);
	// volя┐╜me zarovnя┐╜nя┐╜ vя┐╜sledku (typicky vpravo, jen vyjmecne je vя┐╜hodnя┐╜ vlevo)
	ADC1_AlignConfig(ADC1_ALIGN_RIGHT);
	// nasatvя┐╜me multiplexer na nekterя┐╜ ze vstupnя┐╜ch kanя┐╜lu
	ADC1_Select_Channel(ADC1_CHANNEL_2);
	// rozbehneme AD prevodnя┐╜k
	ADC1_Cmd(ENABLE);
	
	// GPIOD, PIN2
	ADC1_SchmittTriggerConfig(ADC1_SCHMITTTRIG_CHANNEL3,DISABLE);
	// nastavя┐╜me clock pro ADC (16MHz / 4 = 4MHz)
	ADC1_PrescalerConfig(ADC1_PRESSEL_FCPU_D4);
	// volя┐╜me zarovnя┐╜nя┐╜ vя┐╜sledku (typicky vpravo, jen vyjmecne je vя┐╜hodnя┐╜ vlevo)
	ADC1_AlignConfig(ADC1_ALIGN_RIGHT);
	// nasatvя┐╜me multiplexer na nekterя┐╜ ze vstupnя┐╜ch kanя┐╜lu
	ADC1_Select_Channel(ADC1_CHANNEL_2);
	ADC1_Select_Channel(ADC1_CHANNEL_3);
	// rozbehneme AD prevodnя┐╜k
	ADC1_Cmd(ENABLE);
}

uint16_t voltage_reference = 33;
uint16_t devided_battery_voltage = 42;
uint16_t battery_voltage;
uint16_t battery_voltage_final;
uint8_t batterry_really_down = 0;
uint16_t for_loop_i = 0;

INTERRUPT_HANDLER(EXTI_PORTC_IRQHandler, 5) // select current tlatcitko
{
	discharge_setting++;
	if (discharge_setting > 2){
		discharge_setting = 0;
	}
	show_discharge_current(discharge_setting);
}

INTERRUPT_HANDLER(EXTI_PORTB_IRQHandler, 4) // checkuje jestli uzivate potrvdil proud tlacitkem
{
	if (charge_is_on){                            // jakmile PB5 = 1 nabijeni dokonceno, probehne tohle 
		max7219_posli(DIGIT0, 0b00000001);
		max7219_posli(DIGIT1, 0b00000001);
		
		max7219_posli(DIGIT2, 0b00001010); 		
		max7219_posli(DIGIT3, 0b00001010);
		
		max7219_posli(DIGIT4, 0b00001010);
		max7219_posli(DIGIT5, 0b00001010);
		
		max7219_posli(DIGIT6, 0b00001010);
		max7219_posli(DIGIT7, 0b00001010);
		
		charge_is_on = 0;
		charge_off();
		
		if (!second_charge_cycle){
		discharge_on();
		timer_on = 1;
		}
	}
	else {
		discharge_setup();		// to co se provede po potvrzeni proudu 
		charge_setup();
		charge_on();
		charge_is_on = 1;
		GPIO_Init(GPIOB, GPIO_PIN_4, GPIO_MODE_IN_FL_NO_IT);
	}
}

void main(void){
	CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1); // taktovat MCU na 16MHz
	//GPIO_Init(GPIOD, GPIO_PIN_4, GPIO_MODE_OUT_PP_LOW_SLOW); // on-board led
	
	// prochazeci tlacitko setup
	GPIO_Init(GPIOC, GPIO_PIN_3, GPIO_MODE_IN_FL_IT);
	EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOC,EXTI_SENSITIVITY_RISE_ONLY);
	ITC_SetSoftwarePriority(ITC_IRQ_PORTC,ITC_PRIORITYLEVEL_3);
	
	
	// potvrzovaci tlacitko setup
	GPIO_Init(GPIOB, GPIO_PIN_4, GPIO_MODE_IN_FL_IT);
	//EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOB,EXTI_SENSITIVITY_RISE_ONLY);
	//ITC_SetSoftwarePriority(ITC_IRQ_PORTB,ITC_PRIORITYLEVEL_1);
	
	// skonceni nabijeni pin
	GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_IN_PU_IT);
	EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOB,EXTI_SENSITIVITY_RISE_ONLY);
	ITC_SetSoftwarePriority(ITC_IRQ_PORTB,ITC_PRIORITYLEVEL_1);
	
	init_milis();
	_delay_us(100);
	
	setup_ADCs();
	
	charge_discharge_init();
	discharge_off();
	charge_off();
	
	max7219_init();
		
	// zapnu pocitani
	//timer_on = 1;
	
	// prvni zobrazeni discharge proudu, dalя┐╜я┐╜ je v prerusenich
	show_discharge_current(discharge_setting); 
	
	enableInterrupts();
  while (1){//charge animace
		if(((milis() - last_time_charge) > 250) && charge_is_on)
		{
			if(charge_animation == 4){
				charge_animation = 0;
			}
			charge_animate(charge_animation);
			charge_animation++;
			
			last_time_charge = milis();
		}
		
		// controluji, jestli od posledniho pricteni sekundy neuplynula
		// sekunda -> 1000 ms a jestli je timer zapnuty aby pocital
		if(((milis() - last_time_timer) > 1000) && timer_on) // spusti se pod podminkou ze je zapnuty timer a ubehla vice nez sekunda od posledniho zavolani 
		{
			// pokud nekde byla zmenena timer_reset promena na 1 -> timer se resetuje
			if (timer_reset){
				// reset sekund
				time_in_s = 0;
				// nastavym reset zpet na 0
				timer_reset = 0;
			} 
			
			// prictu 1 k casu v sekundach
			time_in_s++;
			
			// every 1s check voltage
			
			// voltage measuring -> stop discharge at 3.0 V
			
			/*for(for_loop_i = 0; for_loop_i < 100; for_loop_i++)
			{
				_delay_us(100);
			}*/
			devided_battery_voltage = ADC_get(ADC1_CHANNEL_2);
			voltage_reference = ADC_get(ADC1_CHANNEL_3);
			battery_voltage = (devided_battery_voltage*10*10);//voltage_reference;
			battery_voltage_final = ((battery_voltage/voltage_reference)*245*2)/100;
			
			max7219_posli(DECODE_MODE, 0b11111101);
			max7219_posli(DIGIT0, (batterry_really_down/10)%10);
			max7219_posli(DIGIT1, 0b00000001);

			if (battery_voltage_final < 300){
				if (batterry_really_down >= 80){
					show_capacity(minutes, discharge_setting);
					discharge_off();
					timer_on = 0;
					charge_on();
					second_charge_cycle = 1;
				}
				batterry_really_down++;
			}
			else{
				if(batterry_really_down == 0){
				batterry_really_down++;
				}
				batterry_really_down--;
			}
			
			// zapis posledniho casu spusteni teto "funkce"
			last_time_timer = milis();
			
			
			// vypocty pro dostani hodnoty v hodinach, minutach, sekundach
			if (!second_charge_cycle){
				hours = time_in_s / 3600;
				display_hours(hours);
				
				minutes = (time_in_s % 3600) / 60;
				display_minutes(minutes);
				
				seconds = time_in_s % 60;
				display_seconds(seconds);
				
				// decimal point blinking
				if (decimal_point){
					decimal_point = 0;
				}
				else{
					decimal_point = 128;
				}
			}
		}
	}
}

#ifdef USE_FULL_ASSERT
void assert_failed(u8* file, u32 line)
{ 
  while (1)
	
  {
  }
}
#endif
