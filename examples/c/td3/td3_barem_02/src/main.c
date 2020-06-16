/*  Copyright 2020 
        codigo basado en el libro Sistemas Empotrados en tiempo real 
        José Daniel Muñoz Frías
1.6.1.  Latencia en sistemas primer plano / segundo plano (Pagina 18)

La tarea mostrada sirve para implantar un reloj. Se ha supuesto que
esta tarea está asociada a la interrupción de un temporizador, el cual se
ha configurado para que interrumpa cada milisegundo. Supóngase ahora
que la función encargada de imprimir la hora tarda en ejecutarse 20 ms.
Como por defecto, mientras se está ejecutando una interrupción, el micro-
procesador mantiene las interrupciones inhabilitadas, durante los 20 ms
en los que se está imprimiendo la hora no se reciben interrupciones y por
tanto no se actualiza el contador de milisegundos. En consecuencia, cada
segundo se atrasan 20 ms, lo cual es un error inaceptable.

nota: en minicom velocidad en 300 bauds -> ctr+a z o

 */



/*==================[inclusions]=============================================*/

#include "board.h"
#include "main.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void initHardware(void)
{
    SystemCoreClockUpdate();
    Board_Init();
}

static void InitSerie(void)
{
    Chip_UART_Init(LPC_USART2);
	Chip_UART_SetBaud(LPC_USART2, 9600);  /* Set Baud rate */
	Chip_UART_ConfigData(LPC_USART2, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT));
	Chip_UART_SetupFIFOS(LPC_USART2, UART_FCR_FIFO_EN | UART_FCR_TRG_LEV3); /* Modify FCR (FIFO Control Register)*/
	Chip_UART_TXEnable(LPC_USART2); /* Enable UART Transmission */
	Chip_SCU_PinMux(7, 1, MD_PDN, FUNC6);              /* P7_1,FUNC6: UART2_TXD */
	Chip_SCU_PinMux(7, 2, MD_PLN|MD_EZI|MD_ZI, FUNC6); /* P7_2,FUNC6: UART2_RXD */
}

static void SeriePuts(char *data)
{
	while(*data != 0)
	{
		while ((Chip_UART_ReadLineStatus(LPC_USART2) & UART_LSR_THRE) == 0) {}
		Chip_UART_SendByte(LPC_USART2, *data);
		data++;
	}
}

static void InitTimer(void)
{
	Chip_RIT_Init(LPC_RITIMER);
	Chip_RIT_SetTimerInterval(LPC_RITIMER,1);
}

static void ImprimeHora(int h, int m, int s)
{
	char cadena [30];

	sprintf (cadena , " Hora: %02d : %02d : %02d\n\r", h, m, s);
	SeriePuts (cadena);
}

void RIT_IRQHandler(void)
{
	static int ms, seg, min, hor;

	ms++;
	if(ms == 1000){
		Board_LED_Toggle(5);
		ms = 0;
		seg++;
		if(seg == 60){
			seg = 0;
			min ++;
			if(min == 60){
				min = 0;
				hor ++;
				if(hor == 24){
					hor = 0;
				}
			}
		}
		ImprimeHora(hor, min, seg);
	}
	Chip_RIT_ClearInt(LPC_RITIMER);
}

/*==================[external functions definition]==========================*/

int main(void)
{
	initHardware(); /* Inicializa el Hardware del microcontrolador */
	InitTimer(); /* Inicializa timer RIT */
	InitSerie(); /* Inicializa puerto serie */

	NVIC_EnableIRQ(RITIMER_IRQn);

	for(;;);
}

/*==================[end of file]============================================*/
