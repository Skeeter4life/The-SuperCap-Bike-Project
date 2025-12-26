/*
 * Motor_Driver.c
 *
 *  Author: Andrew
 */ 


// Phase angle: 120 degrees


#include "../Headers/Includes.h"
#include "../Headers/I2C.h"
#include "../Headers/Motor_Driver.h"
#include "../Headers/Timer_Counter.h"

volatile uint8_t Current_Speed = 0;
volatile Accumulator_State Switching_State = Parallel;

volatile uint8_t Hall_State = 0;
volatile TWI_Status Status;

uint8_t Duty_Cylce = 82;
// uint8_t PWM_Prescaler = 8;

volatile bool Commutation_Busy = true;

volatile bool Motor_Enabled = true; // Some things are volatile because I may use them in an ISR and would rather not forget to make it volatile. Ill optimize once everything works.

PWM_Setup IN_1;
PWM_Setup IN_2;
PWM_Setup IN_3; // Could just use the pins locally for each function, but this makes things a bit more cohesive 

#define SD_U1 PORTD0
#define SD_U2 PORTD4
#define SD_U3 PORTD1

#define IN1_PIN PD6  // U2
#define IN2_PIN PD5  // U1
#define IN3_PIN PB1  // U3

void Update_Commutation(void);

// IN_2 -> P2 IN_1 -> P1 IN_3 -> P3

// In2 and In1 share the same timer

ISR(INT0_vect){
	
	if(Next_I2C_State == TWI_IDLE && (Commutation_Busy == false)){
		Status = TWI_Read(MCP23017_Address, 0x11, &Hall_State, &Update_Commutation); // Clear interrupt flag (read INTCAPB register)
	}else{
		Status = TWI_FAULT;
		//DDRB |= (1 << DDB2);
		//PORTB |= (1 << PORTB2);
	}	
}

void Update_Commutation(void){
	
	Commutation_Busy = true;
	
	const Phase_Logic Commutation_LUT[8] = {
		[0b000] = { Phase_Invalid, Phase_Invalid, Phase_Invalid }, // invalid

		[0b001] = { Phase_B, Phase_C, Phase_A }, // Hall 001 -> B+, C-, Az
		[0b010] = { Phase_C, Phase_A, Phase_B }, // Hall 010 -> C+, A-, Bz
		[0b011] = { Phase_B, Phase_C, Phase_A }, // Hall 011 -> B+, C-, Az

		[0b100] = { Phase_A, Phase_B, Phase_C }, // Hall 100 -> A+, B-, Cz
		[0b101] = { Phase_A, Phase_B, Phase_C }, // Hall 101 -> A+, B-, Cz // Skip?
		[0b110] = { Phase_A, Phase_B, Phase_C }, // Hall 110 -> A+, B-, Cz

		[0b111] = { Phase_Invalid, Phase_Invalid, Phase_Invalid }  // invalid

	};
	
	if(Motor_Enabled == false) return;
	
	if(Status == TWI_FAULT){
		
		Error_Log Motor_Error = {
			
			.Message = "MOTOR T",
			.Time = Global_Timer
			
		};
		
		Log_Error(&Motor_Error);
		
		EICRA &= ~(1 << ISC01); // Disable INT0
		
		PORTD &= ~((1 << SD_U1) | (1 << SD_U2) | (1 << SD_U3)); // Ensure all IR2014's are in shutdown state
		
		Motor_Enabled = false;
		
		return;
		
	}
	
	
	// Error state -> Will add error flag and error handling
		
	// ^ Yeah this really slows things down. Design mistake.
	
	uint8_t Phase_Index = (Hall_State>>3) & 0b111;
	
	if(Phase_Index == 7 || Phase_Index == 0){
		
		Error_Log Motor_Error = {
			
			.Message = "MOTOR H",
			.Time = Global_Timer
			
		};
		
		Log_Error(&Motor_Error);
		
		EICRA &= ~(1 << ISC01); // Disable INT0
		
		PORTD &= ~((1 << SD_U1) | (1 << SD_U2) | (1 << SD_U3)); // Ensure all IR2014's are in shutdown state
		
		Motor_Enabled = false;
		
		return; 
	}
			
	const Phase_Logic* Current_Phase = &Commutation_LUT[Phase_Index]; // Not checking for out of bounds. Mask GPB5-3.
		
	// Phase A is IN_1 or U2, Phase B is IN_2 or U1, Phase C is IN_3 or U3
	
	switch(Current_Phase->High){ // Reminder: IN1 and IN2 share the same timer. Changing 
		
		case Phase_A:
		
			Toggle_PWM(&IN_1, ON);
			DDRD |= (1 << DDD6); // IN1 as output (U2)
			
			break;
		
		case Phase_B:
			
			Toggle_PWM(&IN_2, ON);
			DDRD |= (1 << DDD5); // IN2 (U1)
			
			break;
			
		case Phase_C:
			
			Toggle_PWM(&IN_3, ON);
			DDRB |= (1<< DDB1); // IN3 (U3)
			
			break;
			
		default:
		
			return; // Error state
			
	}
	
	switch(Current_Phase->Low){
		
		case Phase_A:
		
			Toggle_PWM(&IN_1, OFF); // Note: if the COMnX bit(s) are set, the functionality of the PORTx register is overridden
			
			PORTD &= ~(1 << PORTD6); // IN1 digital LOW
			
			DDRD |= (1 << DDD6); // Set output
		
			break;
		
		case Phase_B:
		
			Toggle_PWM(&IN_2, OFF);
			
			PORTD &= ~(1 << PORTD5); // IN2 digital LOW
			
			DDRD |= (1 << DDD5);
					
			break;
		
		case Phase_C:
		
			Toggle_PWM(&IN_3, OFF);
			
			PORTB &= ~(1 << PORTB1); // IN3 digital LOW
			
			DDRB |= (1 << DDB1);
					
			break;
		
		default:
		
			return; // Error state
		
	}
	
	switch(Current_Phase->High_Z){ 
		
		case Phase_A: // Shutdown U1 IR2104
			
			// Toggle_PWM(&IN_1, OFF); 
			
			PORTD &= ~(1 << SD_U2); // SD U2 (Phase A)

			DDRD &= ~(1 << DDD6); // IN1 as an input			
			PORTD |= (1 << SD_U1) | (1 << SD_U3); // Enable PhaseB (IN2), PhaseC (IN3)

			break;
			
		case Phase_B: // Shutdown U2 
			
			// Toggle_PWM(&IN_2, OFF); 
			 
			PORTD &= ~(1 << SD_U1); // SD U1 (Phase B)
			
			DDRD &= ~(1 << DDD5); // IN2 as input
			
			PORTD |= (1 << SD_U2) | (1 << SD_U3); // Enable PhaseA, PhaseC
			
			break;
				
		case Phase_C: // Shutdown U3
			
			// Toggle_PWM(&IN_3, OFF);   
			
			PORTD &= ~(1 << SD_U3); // Shutdown U3 (Phase C)
			
			DDRB &= ~(1 << DDB1); // IN_3 as an input
	
			PORTD |= (1 << SD_U1) | (1 << SD_U2); // Enable PhaseA, PhaseB
			
			break;
			
		default:
		
			return; // Error state
			
	}
	
	Commutation_Busy = false;
	
	return;
}

Motor_Status Init_Motor(void){
	
	DDRB |= (1 << DDB4); // Set MOS_DRIVER as an output
	DDRD |= (1 << DDD7) | (1 << DDD0) | (1 << DDD1) | (1 << DDD4); // Set RELAYS, SD_U1, SD_U3, SD_U2 as outputs
	
	PORTD &= ~((1 << SD_U1) | (1 << SD_U2) | (1 << SD_U3)); // Ensure all IR2014's are in shutdown state
	
	DDRD &= ~((1 << DDD5) | (1 << DDD6));
	DDRB &= ~(1<<DDB1); // Set IN_1, IN_2 and IN_3 as High-Z
	
	TWI_Write(MCP23017_Address, 0x01, 0xFF);         // IODIRB: Set all Port B to inputs
	while(Next_I2C_State != TWI_IDLE);

	TWI_Write(MCP23017_Address, 0x0B, 0b00000100);   // IOCON: INTB is active-low, push-pull
	while(Next_I2C_State != TWI_IDLE);

	TWI_Write(MCP23017_Address, 0x05, 0b00111000);   // GPINTENB Enables interrupt on change for GPIOB5,4,3
	while(Next_I2C_State != TWI_IDLE);

	TWI_Write(MCP23017_Address, 0x09, 0b00000000);   // INTCONB: Ensure pin values are compared against previous pin values (redundant but just to make sure....)
	while(Next_I2C_State != TWI_IDLE);

	uint8_t dummy_read;
	TWI_Read(MCP23017_Address, 0x11, &dummy_read, NULL);  // Read INTCAPB to clear interrupt flag
	while(Next_I2C_State != TWI_IDLE);

	IN_1.Pin = PD6_OC0A;
	
	Timer_Status Timer1_Set = Toggle_PWM(&IN_1, OFF); // Just to make sure the pin can be toggled, and to have the pin initialized in a safe state
	
	Timer_Status Timer1_Config = Configure_PWM(&IN_1, 1, Duty_Cylce); // Testing with 10% duty cycle
	
	if(Timer1_Set == TIMER_FAULT || Timer1_Config == TIMER_FAULT) return Motor_FAULT;
	
	IN_2.Pin = PD5_OC0B; 
	
	Timer_Status Timer2_Set = Toggle_PWM(&IN_2, OFF); // Bugged
	
	Timer_Status Timer2_Config = Configure_PWM(&IN_2, 1, Duty_Cylce);

	if(Timer2_Set == TIMER_FAULT || Timer2_Config == TIMER_FAULT) return Motor_FAULT;
	
	IN_3.Pin = PB1_OC1A;
	
	Timer_Status Timer3_Set = Toggle_PWM(&IN_3, OFF);
	
	Timer_Status Timer3_Config = Configure_PWM(&IN_3, 1, Duty_Cylce);
	
	if(Timer3_Set == TIMER_FAULT || Timer3_Config == TIMER_FAULT) return Motor_FAULT;
	
	sei(); // Ensure interrupts are enabled
		
	EICRA = (1 << ISC01); // The falling edge of INT0 generates an interrupt
	
	DDRD &= ~(1<<DDD2);   // Ensure INT0 is an input

	EIMSK |= (1 << INT0);
	
	//DDRD |= (1 << DDD6); // IN_1 as output
	//Toggle_PWM(&IN_1, ON);
	
	Status = TWI_Read(MCP23017_Address, 0x11, &Hall_State, &Update_Commutation);
	
	if(Status == TWI_FAULT){
		return Motor_FAULT;
	}
	
	return Motor_OK;
	
}

Motor_Status Switch_Accumulator_State(void){
	
	Switching_State = !Switching_State;
	
	PORTD &= ~((1 << PORTD7));
	PORTB &= ~((1 << PORTB4));
	
	uint64_t Current_Tick = System_Ticks[Global_Timer];
	
	if(Timer_Unit[Global_Timer] != u_MiliSeconds || Timer_Step[Global_Timer] != 1 || Timer_Mode[Global_Timer] != TIMER_CTC) return Motor_FAULT;
	while(System_Ticks[Global_Timer] < (Current_Tick + 12)); // 10 ms relay operating time (allow for 30% error)
	
	// Accumulator disconnected
	
	switch(Switching_State){
		
		case Parallel:
		
			PORTD |= (1 << PORTD7);  // Set RELAY signal high
			
			break;
		
		case Series:
		
			PORTB |= (1 << PORTB4); // Set MOS_DRIVER signal high
			
			break;
		
		default:
		
			return Motor_FAULT;
		
	}
	
	return Motor_OK;
	
}