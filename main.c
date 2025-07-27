#define  F_CPU 7372800UL
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Khai bao led7seg
#define DATA_A PC0			// khai bao led7seg kenh A
#define CLK_A  PC1
#define DATA_B PC3			// khai bao led7seg kenh B
#define CLK_B  PC2
#define CHOT   PC4

// khai bao Den giao thong
#define A_G	   PA0		// led xanh kenh A
#define A_Y	   PA1
#define A_R	   PA2

#define B_G	   PA3			// led xanh kenh B
#define B_Y	   PA4
#define B_R	   PA5

unsigned char arr[4] = {'R','R','G','Y'}; //bien manual
int manual_A = 0;
int manual_B = 2;		//bien cua manual

int mode = 0;	//auto
/*unsigned char decode[] = {0x81, 0xCF, 0x92, 0x86, 0xCC,		//Proteus
						  0xA4, 0xA0, 0x8F, 0x80, 0x84,
						  0xff};*/
						  
unsigned char decode[] = {0x03, 0x9F, 0x25, 0x0D, 0x99, 
						   0x49, 0x41, 0x1F, 0x01, 0x09,
						   0xff};

//======================Ham dich bit===================
void shiftbit_A(unsigned int n){
	int j, t;
	for (j = 0; j < 8; j++){
		t = (decode[n] >> j) & 0x01;
		if(t == 0x01){
			PORTC |= (1<<DATA_A);
		}
		else{
			PORTC &= ~(1<<DATA_A);
		}
		PORTC &=~(1<<CLK_A);
		PORTC |=(1<<CLK_A);
	}
}

void shiftbit_B(unsigned int n){
	int j, t;
	for (j = 0; j < 8; j++){
		t = (decode[n] >> j) & 0x01;
		if(t == 0x01){
			PORTC |= (1<<DATA_B);
		}
		else{
			PORTC &= ~(1<<DATA_B);
		}
		PORTC &=~(1<<CLK_B);
		PORTC |=(1<<CLK_B);
	}
}

//======================Ham hien thi============================
void hienthi_A (unsigned int num){
	int hangchuc = num / 10;			//chia lay phan nguyen
	int donvi = num - hangchuc*10;
		
	//Dich bit hang don vi truoc roi den hang chuc
	shiftbit_A(donvi);
	shiftbit_A(hangchuc);
	
	//Hien thi
	PORTC &=~(1<<CHOT);
	PORTC |=(1<<CHOT);
}

void hienthi_B (unsigned int num){
	int hangchuc = num / 10;			//chia lay phan nguyen
	int donvi = num - hangchuc*10;
	
	//Dich bit hang don vi truoc roi den hang chuc
	shiftbit_B(donvi);
	shiftbit_B(hangchuc);

	//Hien thi
	PORTC &=~(1<<CHOT);
	PORTC |=(1<<CHOT);
}
//=======================Ham den giao thong=========================
void den(char type, char color){	//type A, B; color = r, y, g
	if (type == 'A'){
		PORTA &=~((1<<A_R)|(1<<A_Y)|(1<<A_G));	//clear den
		switch(color){
			case 'R':				//Do - Red
			PORTA |= (1<<A_R);
			break;
			case 'Y':				//Vang - Yellow
			PORTA |= (1<<A_Y);
			break;
			case 'G':				//Xanh - Green
			PORTA |= (1<<A_G);
			break;
		}
	}
	else{ //Neu la loai B
		PORTA &=~((1<<B_R)|(1<<B_Y)|(1<<B_G));	//clear den
		switch(color){
			case 'R':				//Do - Red
			PORTA |= (1<<B_R);
			break;
			case 'Y':				//Vang - Yellow
			PORTA |= (1<<B_Y);
			break;
			case 'G':				//Xanh - Green
			PORTA |= (1<<B_G);
			break;
		}
	}
}

//======================Auto_Traffic_Controller==================
void auto_traffic_controller(int do_time, int xanh_time){
	int vang_time = do_time - xanh_time;
		for(int i = do_time; i >= vang_time + 1; i--){		//A_Do - B_Xanh 
			if(manual_A == 0 && manual_B == 2){		//!!
				den('A',arr[manual_A]);
				den('B',arr[manual_B]);
				hienthi_A(i);
				hienthi_B(i-vang_time);
				_delay_ms(1000);
			}
			else if(manual_A == 2 && manual_B == 0){
				den('A',arr[manual_A]);
				den('B',arr[manual_B]);
				hienthi_B(i);
				hienthi_A(i-vang_time);
				_delay_ms(1000);
			}
			else{break;}	
		}

		for(int i = vang_time; i >= 1; i--){				//A_Do - B_Vang
			if(manual_A == 1 && manual_B == 3){		//!!
				den('A',arr[manual_A]);
				den('B',arr[manual_B]);
				hienthi_A(i);
				hienthi_B(i);
				_delay_ms(1000);
			}
			else if(manual_A == 3 && manual_B == 1){		//A_Vang - B_Do
				den('A',arr[manual_A]);
				den('B',arr[manual_B]);
				hienthi_A(i);
				hienthi_B(i);
				_delay_ms(1000);
			}
			else{break;}
		}
		
		manual_A++;
		manual_B++;
		if (manual_A > 3){manual_A = 0;}
		if(manual_B > 3){manual_B = 0;}
}

//================Ngat chuyen che do manual===================
ISR(INT4_vect){	
	for(int i = 0; i < 2; i++){
		PORTB |= (1 << PB6);		// led on
		PORTB &= ~(1 << PB7);		// buzzer on
		_delay_ms(500);
			
		PORTB &= ~(1 << PB6);		// led off
		PORTB |= (1 << PB7);		// buzzer off
		_delay_ms(500);	
		};

	//Clear number
	shiftbit_A(10);
	shiftbit_A(10);
	shiftbit_B(10);
	shiftbit_B(10);
	//Hien thi
	PORTC &=~(1<<CHOT);
	PORTC |=(1<<CHOT);
	
	while(bit_is_clear(PINE,4)){			//Nhan switch chuyen che do de thoat	
			den('A',arr[manual_A]);
			den('B',arr[manual_B]);
			
			if(bit_is_clear(PINE,5)){				//Dao trang thai de
				manual_A++;
				manual_B++;
				if (manual_A > 3){manual_A = 0;}
				if(manual_B > 3){manual_B = 0;}
				_delay_ms(500);
			}
			_delay_ms(20);
	}
	for(int i = 0; i < 2; i++){
		PORTB |= (1 << PB6);		// led on
		PORTB &= ~(1 << PB7);		// buzzer on
		_delay_ms(500);
		
		PORTB &= ~(1 << PB6);		// led off
		PORTB |= (1 << PB7);		// buzzer off
		_delay_ms(500);
	};
	
	EIFR = 0xFF; //Kich co trang thai de thoat khoi ngat	
		
}

//======================Ham main=======================
int main(void){
	// Khai bao led7seg
	DDRC |= (1 << DATA_A)|(1 << CLK_A)|(1 << DATA_B)|(1 << CLK_B)|(1 << CHOT);	
	
	// Khai bao Den Giao Thong	
	DDRA |= (1 << A_G)|(1 << A_Y)|(1 << A_R)|(1 << B_G)|(1 << B_Y)|(1 << B_R);
	
	// khai bao nut chuyen mode auto/manual, nut chuyen den trong manual va nut set mode
	DDRE &= ~((1 << PE4)|(1 << PE5));
	PORTE |= (1 << PE4)|(1 << PE5);			// dien tro noi keo len
	
	//Khai bao Buzzer + Led
	DDRB |= (1<<PB6)|(1<<PB7);		
	PORTB |= (1<<PB7);
	
	//Ngat
	EICRB |= (1<<ISC41);			//Chuyen mode kich canh xuong
	EIMSK |= (1<< INT4);			//PE4
	sei();
	
	while(1){
		while(bit_is_set(PINE, 4)){
			auto_traffic_controller(12,10);		//Thoi gian do - thoi gian xanh = thoi gian vang
		}
	}
}
