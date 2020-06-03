/* este codigo NO esta en el libro de Sistemas Empotrados en tiempo real
 * pero el objetivo es intriducir a FreeRTOS usando solo una  tarea que 
 * imprime por el puerto serie, e invocando al planificador
 *
 *  - xTaskCreate()
 *  - vTaskStartScheduler()
 *
 * esto hará un busy waiting ... o podemos usar :
 *  - vTaskDelay(500 / portTICK_RATE_MS); // 500 ms en este caso
 *
 *  */
 /*==================[inclusions]=============================================*/

#include "board.h"
#include "chip.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "main.h"

/*==================[macros and definitions]=================================*/

#define PRIO_IMP_HORA 1
#define TAM_PILA 1024

typedef struct {
uint8_t hor;
uint8_t min;
uint8_t seg;
uint16_t useg;
}HORA;

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/** @brief hardware initialization function
 *	@return none
 */
//static void initHardware(void);

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

static HORA hora_act ={0,0,0,0};

/*==================[internal functions definition]==========================*/

static void initHardware(void)
{
    SystemCoreClockUpdate();
    Board_Init();
 
}

static void InitSerie(void)
{
    Chip_UART_Init(LPC_USART2);
	Chip_UART_SetBaud(LPC_USART2, 115200);  /* Set Baud rate */
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

static void ImprimeHora(void * a)
{
	HORA copia_hora ;
	char cadena [10];
	
	while (1){
			copia_hora = hora_act ;
			sprintf (cadena , " %02d: %02d: %02d\n", copia_hora.hor, copia_hora.min, copia_hora.seg );
			SeriePuts (cadena); 
			//DELAY
			vTaskDelay(500 / portTICK_RATE_MS);
	}
}


void RIT_IRQHandler(void)
{
	hora_act.useg++;
	if (hora_act.useg == 1000){
		hora_act.useg = 0;
		hora_act.seg++;
		Board_LED_Toggle(5); //titila "LED 3" ( verde )
		if( hora_act.seg == 60){
			hora_act.seg = 0;
			hora_act.min++;
			if( hora_act.min == 60){
				hora_act.min = 0;
				hora_act.hor++;
				if( hora_act.hor == 24){
					hora_act.hor = 0;
				}
			}
		}
	}
	/* Borra el flag de interrupción */
	Chip_RIT_ClearInt(LPC_RITIMER);

}

/*==================[external functions definition]==========================*/

int main(void)
{
	initHardware(); /* Inicializa el Hardware del microcontrolador */
	InitTimer();
	InitSerie();
	//InitQueSeYo ();
	/* Se crean las tareas */
	xTaskCreate(ImprimeHora, (const char *)"ImpHora", TAM_PILA, NULL, PRIO_IMP_HORA, NULL );

	NVIC_EnableIRQ(RITIMER_IRQn); //comentar que hace esta linea .....
	vTaskStartScheduler(); /* y por último se arranca el planificador . */
}

/*==================[end of file]============================================*/
