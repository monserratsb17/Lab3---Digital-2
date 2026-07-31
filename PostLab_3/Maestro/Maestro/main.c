/* Rodrigo García - 23387
   Monserrat Samayoa - 23431*/

#define F_CPU 16000000UL  

#include <avr/io.h>         
#include <util/delay.h>     
#include "SPI.h"            
#include "UART.h"          
#include "UART2.h"         

uint8_t valor_spi = 0;      // Variable que almacena el valor recibido por SPI
float volt1 = 0;            // Voltaje leído del canal 1 (potenciómetro 1)
float volt2 = 0;            // Voltaje leído del canal 2 (potenciómetro 2)
int numero = 0;             // Variable general para números (sin uso directo en main)
unsigned char dato = 0;     // Variable auxiliar (sin uso directo en main)
uint8_t leds_val = 0;       // Valor que se envía al puerto de LEDs

void LEDS_PORT(uint8_t mascara);  // Declaración de función que controla los LEDs
unsigned char pines[8] = {2, 3, 4, 5, 6, 7, 8, 9};  // Mapeo lógico de pines para los LEDs

int main(void)
{
	// Configura pines PD2 a PD7 como salidas (6 bits del puerto D)
	DDRD |= (1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7);

	// Configura pines PB0, PB1 y PB2 como salidas (por ejemplo, SPI y control LED)
	DDRB |= (1<<0)|(1<<1)|(1<<2); // PB2 se usa como CS para SPI

	PORTD = 0;
	PORTB = 0;

	SPI_init(SPI_MASTER_OSC_DIV16, SPI_DATA_ORDER_MSB, SPI_CLOCK_IDLE_HIGH, SPI_CLOCK_FIRST_EDGE);

	USART_Init(103);

	LEDS_PORT(0);

	char opcion; 

	while (1)
	{
		// Mostrar menú principal por UART
		UART_Char('\n');
		UART_Write("=============== MENU ============\r\n");
		UART_Write("1. Ver voltajes de los potenciometros\r\n");
		UART_Write("2. Ingresar numero para mostrar con LEDs y SPI\r\n");
		UART_Write("Selecciona una opcion: ");

		// Espera una opción del usuario por UART
		opcion = USART_Receive();
		UART_Char(opcion);  // Eco del carácter ingresado
		UART_Char('\n');

		// Opción 1: Medición de voltajes en bucle
		if (opcion == '1') {
			UART_Write("\r\nPresiona 'x' para salir de las lecturas.\r\n");

			while (1) {
				if (UCSR0A & (1 << RXC0)) {
					char salida = UDR0; 
					if (salida == 'x') break; 
				}

				// Lectura del voltaje del canal 1 (pot 1)
				PORTB &= ~(1<<2);       // Baja CS (selecciona esclavo SPI)
				SPI_Write('a');         
				valor_spi = SPI_Read(); 
				volt1 = 5.0 * valor_spi / 255.0; 
				PORTB |= (1<<2);       

				// Lectura del voltaje del canal 2 (pot 2)
				PORTB &= ~(1<<2);       
				SPI_Write('b');         
				valor_spi = SPI_Read(); 
				volt2 = 5.0 * valor_spi / 255.0; 
				PORTB |= (1<<2);        

				// Mostrar los voltajes en UART
				UART_Char('\n');
				UART_Write("Volt 1: ");
				UART_Var(volt1);
				UART_Write(" V\r\n");

				UART_Write("Volt 2: ");
				UART_Var(volt2);
				UART_Write(" V\r\n");

				_delay_ms(500);
			}
		}

		// Opción 2: Ingreso y visualización de un número
		else if (opcion == '2') {
			_delay_ms(1000); // Pausa para evitar conflictos de lectura

			while (UCSR0A & (1 << RXC0)) {
				char temp10 = UDR0; 
			}

			UART_Write("\r\nIngresa un numero (0-255): ");
			uint8_t numero = USART_GetNum();  // Función que recibe un número completo desde UART

			// Confirma el número ingresado
			UART_Write("\r\nNumero ingresado: ");
			UART_Num(numero);
			UART_Write("\r\n");

			// Muestra el número en los LEDs
			LEDS_PORT(numero);

			// Envía el número por SPI
			PORTB &= ~(1<<2);       
			SPI_Write(numero);       
			valor_spi = SPI_Read();  
			PORTB |= (1<<2);         
		}

		// Manejo de opción inválida
		else {
			UART_Write("Opcion invalida. Intenta nuevamente.\r\n");
		}
	}
}

// Función para controlar el encendido/apagado de LEDs según una máscara de bits
void LEDS_PORT(uint8_t mascara) {
	for(int i = 0; i < 8; i++) {  
		if (mascara & (1 << i)) { 
			uint8_t pin = pines[i]; 
			if (pin >= 8 && pin <= 13) {
				PORTB |= (1 << (pin - 8)); 
				} else if (pin <= 7) {
				PORTD |= (1 << pin); 
			}
			} else { // Si el bit está en 0, apaga el LED correspondiente
			uint8_t pin = pines[i];
			if (pin >= 8 && pin <= 13) {
				PORTB &= ~(1 << (pin - 8)); // Apaga LED en PORTB
				} else if (pin <= 7) {
				PORTD &= ~(1 << pin); // Apaga LED en PORTD
			}
		}
	}
}