/*******************HeaderFiles*****************/
#include<avr/io.h>
#include<avr/interrupt.h>
#include <LiquidCrystal.h>
#include<util/delay.h>

#define SET_BIT(PORT,BIT) PORT|= (1<<BIT)
#define CLR_BIT(PORT,BIT) PORT&= ~(1<<BIT)

/*****************GlobalVariable****************/
unsigned int HVAC_flag=0;      //flag to turn on module
unsigned int Engine=0;
unsigned int SeatBelt_flag=0;
float distance1=0;
float distance2=0;
double temperature;       //current temperature
LiquidCrystal lcd(12,11,9,10,6,7);

/****************FunctionDeclaration*************/
void PortInitialize();    //Initialization of ports
int read_user_value();    //Function to read desired temperature set by the user
double temperature_read();  //Function to read current temperature
void set_fanspeed(double actual_temperature,int  desired_temperature);   //Function to set fanspeed based on user's input

/****************main function*******************/
int main()
{
  Serial.begin(9600);
 	PortInitialize();
 	while(1)
    {
		while(Engine)
      	{
      		if(HVAC_flag)
      		{
       	 	double actual_temperature=temperature_read(); 
       	 	int desired_temperature=read_user_value();
       	 	set_fanspeed(actual_temperature,desired_temperature);
      		}
      		if(!HVAC_flag)
      		{
         	double actual_temperature=temperature_read();
         	lcd.setCursor(0,1);
         	lcd.print("HVAC-OFF    ");
      		}
          	if(!SeatBelt_flag)
            {
            PORTD |= (1<<PD1); //Buzzer On
            }
          	if(SeatBelt_flag)
            {
            PORTD &= ~(1<<PD1); //Buzzer Off
            }
          //airbag
          	SET_BIT(DDRD,PD5);
      		SET_BIT(PORTD,PD5);
      		_delay_ms(5);
      		CLR_BIT(PORTD,PD5);
      		CLR_BIT(DDRD,PD5);
      		int time_duration1 = pulseIn(PD5,HIGH);
      		distance1=0.343*time_duration1*0.5*0.1;
            if(distance1 < 100)
            {
              SET_BIT(PORTC, PC2);//Led On
            }
            if(distance1 > 100)
            {
              CLR_BIT(PORTC, PC2);//Led Off
            }
          //powerwindow
      		SET_BIT(DDRD,PD4);
      		SET_BIT(PORTD,PD4);
      		_delay_ms(5);
      		CLR_BIT(PORTD,PD4);
      		CLR_BIT(DDRD,PD4);
      		int time_duration2 = pulseIn(PD4,HIGH);
      		distance2=0.343*time_duration2*0.5*0.1;
            if(distance2 > 100)
            {
            SET_BIT(PORTB,PB5);//motor
            }
            if(distance2 < 100)
            {
            CLR_BIT(PORTB,PB5);//motor
            }
        }/*
        while(!Engine)
        {
         //close everything
         
        }*/
     }
}
void PortInitialize()
{
  sei();
  EICRA |= (1 << ISC10);   // set INT1 to to any logical change
  EIMSK |= (1 << INT1);
  
  sei();
  EICRA |= (1 << ISC00);   // set INT0 to to any logical change
  EIMSK |= (1 << INT0);
  
  sei();
  PCICR|=(1<<PCIE0);       //local interrupt enable
  PCMSK0|=(1<<PCINT0);     //enable pin in mask register
  
  SET_BIT(DDRB,PB5);       //motor

  CLR_BIT(DDRD,PD4);       //Echopin powerwindow
  CLR_BIT(DDRD,PD5);       //Echopin AirBag
  CLR_BIT(DDRB,PB0);       //inputConf buzzer
  CLR_BIT(DDRD,PD2);       //inputConf Engine

  DDRD&=!(1<<PD3);         //switch to turn on module
  DDRC&=!(1<<PC0);         //temperature sensor
  DDRC&=!(1<<PC1);         //potentiometer
  DDRD|= (1<<PD1);         //Buzzer
  CLR_BIT(PORTD,PD1);
  DDRC|= (1<<PC2);         //Led
  lcd.begin(16,2);
}

/****************FunctionsDefinations****************/

//Function to read current temperature
double temperature_read()
{
    ADMUX=0x40;
    ADCSRA=0x80;
    unsigned int value=0;
    ADCSRA|=(1<<ADSC);
    loop_until_bit_is_clear (ADCSRA, ADSC);
    unsigned int low,high;
    low=ADCL;
    high=ADCH;
    value=high;
    value=value<<8;
    value=value|low;
    double temperature;
    temperature = (double)value / 1024;          //find percentage of input reading
    temperature = temperature * 5;               //multiply by 5V to get voltage
    temperature = temperature - 0.5;             //Subtract the offset
    temperature = temperature * 100;
  	lcd.setCursor(0,0);
  	lcd.print("A:");
    if(temperature<0)
    {
      lcd.setCursor(3,0);
      lcd.print(temperature);
    }
    else
    {
      lcd.setCursor(3,0);
      lcd.print(temperature);
    }
  	lcd.setCursor(6,0);
  	lcd.print("  ");    
    return temperature;
}

//Function to read desired temperature set by the user
int read_user_value()    
{
    ADMUX=0x41;
    ADCSRA=0x80;
    unsigned int value=0;
    ADCSRA|=(1<<ADSC);
    loop_until_bit_is_clear (ADCSRA, ADSC);
    unsigned int low,high;
    low=ADCL;
    high=ADCH;
    value=high;
    value=value<<8;
    value=value|low;
    int array_index=value/68;
    int temperature_array[]= {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,30};
    int desired_temperature=temperature_array[array_index];
    lcd.setCursor(7,0);
    lcd.print(" D:");
	lcd.setCursor(10,0);
    lcd.print(desired_temperature);
    return desired_temperature;
}

//Function to set fanspeed based on user's input
void set_fanspeed(double actual_temperature,int  desired_temperature)   
{
    int temperature=actual_temperature;
    int set_temperature=desired_temperature;
    int difference;
    if(set_temperature<temperature)
    {
        difference=temperature-set_temperature;
        if(difference>4)
        {
            _delay_ms(150);
            lcd.setCursor(0,1);
          	lcd.print("AC      ");
            lcd.setCursor(8,1);
          	lcd.print("HIGH");
            _delay_ms(150);
        }
        else
        {
            _delay_ms(150);
            lcd.setCursor(0,1);
          	lcd.print("AC      ");
            lcd.setCursor(8,1);
          	lcd.print("LOW ");
            _delay_ms(150);
        }
    }
    if(set_temperature>temperature)
    {
        difference=set_temperature-temperature;
        if(difference>4)
        {
            _delay_ms(150);
            lcd.setCursor(0,1);
          	lcd.print("HEATER  ");
            lcd.setCursor(8,1);
          	lcd.print("HIGH");
            _delay_ms(150);
        }
        else
        {
            _delay_ms(150);
            lcd.setCursor(0,1);
          	lcd.print("HEATER  ");
            lcd.setCursor(8,1);
          	lcd.print("LOW ");
            _delay_ms(150);
        }
    }
}

//Interrupt0 Engine
ISR(INT0_vect)
{
  Engine=!Engine;
}
//Interrupt1 HVAC
ISR(INT1_vect)
{
  HVAC_flag=!HVAC_flag;
}
//Interrupt PinChange0
ISR(PCINT0_vect)
{
  SeatBelt_flag=!SeatBelt_flag;
}
