/*  Copyright 2020 - FreeRTOS Kernel V10.0.1

    codigo basado en el libro Sistemas Empotrados en tiempo real 
    José Daniel Muñoz Frías
4.8.1. Rutinas de atención a interrupción en FreeRTOS

En FreeRTOS existen dos tipos de llamadas al
sistema operativo: las normales y las diseñadas para ser llamadas desde
una rutina de atención a interrupción.

Las funciones diseñadas para ser llamadas desde las rutinas de aten-
ción a interrupción tienen dos diferencias con respecto a las normales:
- No pueden bloquearse, al estar pensadas para ser llamadas desde una
interrupción.
- No producen cambios de contexto, aunque la escritura o recepción de
datos de la cola despierten a una tarea más prioritaria que la tarea
que estaba ejecutándose antes de que se produjese la interrupción.
En este caso, el cambio de contexto es necesario hacerlo “a mano” al
finalizar la ejecución de la rutina de interrupción, de forma que desde
la interrupción se vuelva a la rutina recién despertada.

Ejemplo de manejo de colas desde una rutina de atención a
interrupción usando FreeRTOS (Pagina 120 )

En dicho ejemplo se usa una cola para comunicar
la rutina de interrupción del puerto serie con la tarea de primer plano. La
rutina de atención a la interrupción se limita a copiar el carácter recibido
de la UART en la cola. La tarea de primer plano se encarga de verificar si
hay caracteres nuevos en la cola en cada iteración del bucle de scan. Si hay
un carácter nuevo, lo saca de la cola y lo copia en una cadena denominada
mensaje . Cuando recibe un mensaje completo, indicado por la recepción del
carácter de retorno de carro, se procesa dicho mensaje


NOTA: deshabilitar en minicom control de flujo por hardware  !!!!
 */

/*==================[inclusions]=============================================*/

#include "board.h"
#include "chip.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"


/*==================[macros and definitions]=================================*/

#define TAM_COLA 10
#define TAM_PILA 512
#define PRIO_PROC_SER 2

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

QueueHandle_t cola_rec ; /* Cola para recibir */

/*==================[internal functions definition]==========================*/

static void InitHardware(void)
{
    SystemCoreClockUpdate();
    Board_Init();
}

static void InitSerie(void)
{

	/* Primero se crea la cola */
	cola_rec = xQueueCreate (TAM_COLA , sizeof (char));
	if( cola_rec == NULL ){
		Board_LED_Set(4, TRUE); /* prende "LED 2" (rojo) indica error Fatal */
		while (1); /* Se queda bloqueado el sistema hasta que
					venga el técnico de mantenimiento */
	}
    Chip_UART_Init(LPC_USART2);
	Chip_UART_SetBaud(LPC_USART2, 115200);  /* Set Baud rate */
	Chip_UART_ConfigData(LPC_USART2, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT));
	Chip_UART_SetupFIFOS(LPC_USART2, UART_FCR_FIFO_EN | UART_FCR_TRG_LEV3); /* Modify FCR (FIFO Control Register)*/
	Chip_UART_TXEnable(LPC_USART2); /* Enable UART Transmission */
	Chip_SCU_PinMux(7, 1, MD_PDN, FUNC6);              /* P7_1,FUNC6: UART2_TXD */
	Chip_SCU_PinMux(7, 2, MD_PLN|MD_EZI|MD_ZI, FUNC6); /* P7_2,FUNC6: UART2_RXD */
	
	Chip_UART_IntEnable(LPC_USART2, UART_IER_RBRINT);
    NVIC_SetPriority(USART2_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
	NVIC_EnableIRQ(USART2_IRQn);
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

static void ProcesaMensaje(char *data)
{
	SeriePuts(data); //hace eco
}

void ProcesaRecSerie (void * pvParameters)
{
	static char mensaje [100];
	static uint8_t indice = 0;
	char car_rec ;
	
	while (1){		

		if((xQueueReceive (cola_rec , &car_rec ,
			(portTickType) 0xFFFFFFFF )) == pdTRUE){  //portMAX_DELAY espera sin timeout
			/* Se asegura comparando con pdTRUE que ha recibido un carácter de la cola.
				Se almacena */
			mensaje[indice] = car_rec;
            //printf("%c\r\n",car_rec);
			if(mensaje[indice] == '\r'){
				/* El \n indica el final del mensaje */
				mensaje[indice+1] = '\n'; //agrego un new line al carriage return
				mensaje[indice+2] = '\0';
				ProcesaMensaje(mensaje);
				indice = 0;
				Board_LED_Toggle(5); //cambio estado "LED 3" (verde)
			}else{
				indice ++;
			}
		}
	}
}

void UART2_IRQHandler(void)
{
	BaseType_t xTaskWokenByPost = pdFALSE;
	char car_recibido ;

	if((Chip_UART_ReadLineStatus(LPC_USART2) & UART_LSR_RDR) == 1){
		/* Llegó un carácter . Se lee del puerto serie */
		car_recibido = Chip_UART_ReadByte(LPC_USART2);
		Board_LED_Toggle(3); //cambio estado LED amarillo
		/* Y se envía a la cola de recepción */
		xQueueSendFromISR(cola_rec, &car_recibido, &xTaskWokenByPost);
        //validar si retorno con errQUEUE_FULL o pdPASS
		if( xTaskWokenByPost == pdTRUE ){
			portYIELD_FROM_ISR( xTaskWokenByPost );
            /* Si el envío a la cola ha despertado una tarea ,
            se fuerza un cambio de contexto */
		}
	}
}

/*==================[external functions definition]==========================*/

int main(void)
{
	InitHardware(); /* Inicializa el Hardware del microcontrolador */
	InitSerie();

	/* Se crean las tareas */
	xTaskCreate(ProcesaRecSerie, (const char *)"ProcSerie", TAM_PILA, NULL, PRIO_PROC_SER, NULL );

	vTaskStartScheduler(); /* y por último se arranca el planificador . */
}

/*==================[end of file]============================================*/
