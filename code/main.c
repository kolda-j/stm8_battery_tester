// Vzorový projekt pro STM8S103F
#include "stm8s.h"
#include "delay.h"
#include "milis.h"
#include "spse_stm8.h"


#define discharge_port GPIOD
#define discharge_pin GPIO_PIN_3

#define discharge_chip_select_port GPIOD
#define discharge_chip_select_pin GPIO_PIN_6

#define discharge_increment_port GPIOD
#define discharge_increment_pin GPIO_PIN_5

#define discharge_up_down_port GPIOD
#define discharge_up_down_pin GPIO_PIN_4

#define charge_port GPIOD
#define charge_pin GPIO_PIN_2

#define charge_chip_select_port GPIOA
#define charge_chip_select_pin GPIO_PIN_1

#define charge_increment_port GPIOA
#define charge_increment_pin GPIO_PIN_2

#define charge_up_down_port GPIOA
#define charge_up_down_pin GPIO_PIN_3

uint16_t charge_counter = 0;
uint16_t discharge_counter = 0;
uint16_t d = 2; // nastaveni vybijeciho proudu
									// 2=100mA, 5=253mA, 20=1009mA
uint16_t ch = 12; //nastaveni nabijeciho proudu 
                                    //  =100mA, =250mA, =1000mA, (5000mah 3,5V baterie pro nabijeni)

void init(void)
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
}

void charge_down_resistance(void)
{
	GPIO_WriteLow(charge_up_down_port, charge_up_down_pin);
	GPIO_WriteLow(charge_increment_port, charge_increment_pin);
	_delay_us(100);
	GPIO_WriteHigh(charge_increment_port, charge_increment_pin);
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
 
void discharge_setup(void)
{
    while (discharge_counter < 100)
    {
        discharge_up_resistance();
        discharge_counter++;
    } 
    discharge_counter = 0;     
    while (discharge_counter < d)
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

 
void main(void){
CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1); // taktovat MCU na 16MHz
init_milis();
_delay_us(100);

init();
discharge_off();
charge_on();
discharge_setup();
charge_setup();

}


#ifdef USE_FULL_ASSERT
void assert_failed(u8* file, u32 line)
{ 
  while (1)
	
  {
  }
}
#endif
