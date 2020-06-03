/* Copyright 2017
	codigo basado en el libro Sistemas Empotrados en tiempo real 
4.5.1. Ejemplo (Pagina 96) (Ejemplo usando FreeRTOS) y modificación de
4.5.3. Semáforos usados para sincronizar tareas (Pagina 101)
Para ilustrar el funcionamiento de los semáforos, supóngase que en un
sistema se necesita enviar la hora constantemente (en realidad sólo cuando
cambie) por el puerto serie y el estado de 8 entradas digitales. Por tanto,
el puerto serie se comparte ahora por dos tareas: ImprimeHora() para imprimir
la hora y EnviaEntradas() para imprimir el estado de las entradas.
Será por tanto necesario arbitrar el acceso al puerto serie por ambas tareas,
por ejemplo mediante un semáforo (sem_serie)
Se usa un semáforo adicional (sem_hora) para que ImprimeHora() se quede
bloqueado mientras no cambia la hora. Lo incrementa la rutina RIT_IRQHandler()
sem_serie 	->	exclusion mutua para seccion critica
sem_hora	->	serializar para imprimir luego de cambiar hora
nota: en minicom agregar Add Carriage Ret -> ctr+a z u
      velocidad en 115200 -> ctr+a z o
 */

/*==================[inclusions]=============================================*/

#include "board.h"
#include "chip.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "semphr.h"

#include "main.h"

/*==================[macros and definitions]=================================*/

#define PRIO_IMP_HORA 2 //mas prioritaria
/* La tarea EnviaEntrada se ejecuta permanentemente. Luego que escribe el puerto serie,
 y libera el semáforo, por lo que la area ImprimeHora pasará al estado “lista” y 
 como tiene mayor prioridad que EnviaEntradas, se comenzará a ejecutar
 que pasaría si la tarea EnviaEntrada tuviera mas prioridad que la ImprimeHora ?
 */
#define PRIO_ENV_ENTR 1
#define TAM_PILA 1024

typedef struct {
uint8_t hor;
uint8_t min;
uint8_t seg;
}HORA;

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/** @brief hardware initialization function
 *	@return none
 */
static void initHardware(void);

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

xSemaphoreHandle sem_serie;
xSemaphoreHandle sem_hora;
static HORA hora_act ={0,0,0};

/*==================[internal functions definition]==========================*/

static void initHardware(void)
{
    SystemCoreClockUpdate();
    Board_Init();
 
}

uint8_t LeeEntradas(void)
{
	return Buttons_GetStatus();
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
	Chip_RIT_SetTimerInterval(LPC_RITIMER,1000);
}

static void ImprimeHora(void * a)
{
	HORA copia_hora ;
	char cadena [10];
	extern xSemaphoreHandle sem_serie ;
	
	while (1){
		if( xSemaphoreTake (sem_hora , ( portTickType ) 2000 ) == pdTRUE ){
			/* Se bloquea hasta que llegue la interrupción de tiempo */
//			DisableInt();  
			copia_hora = hora_act ;
//			EnableInt ();
			sprintf (cadena , " %02d: %02d: %02d\n", copia_hora.hor, copia_hora.min, copia_hora .seg );
			if( xSemaphoreTake (sem_serie, ( portTickType ) 1000 ) == pdTRUE ){
				SeriePuts (cadena); /* Se tiene el semáforo : se puede acceder al puerto serie */
				xSemaphoreGive ( sem_serie ); /*Se suelta el semáforo */
			}else{
				/* Después de 1000 ticks no se ha obtenido el
				semáforo . Se podría dar un aviso o
				simplemente no hacer nada como en este caso 
				o que tooglee un led rojo */
				Board_LED_Toggle(4); //cambio estado "LED 2" (rojo)

			}
		}
	}
}

static void EnviaEntradas(void * a)
{
	char cadena [100]; /* Guarda el mensaje a transmitir */
	uint8_t entradas;
	static uint8_t entradas_ant = 0;
	extern xSemaphoreHandle sem_serie ;

	while (1){
		entradas = LeeEntradas ();
		if( entradas_ant != entradas ){ /* Sólo imprime si cambian las entradas */
			sprintf (cadena , "Entradas: %x\n", entradas );
			if( xSemaphoreTake (sem_serie , ( portTickType ) 1000) == pdTRUE ){
				/* Se tiene el semáforo : se puede acceder al puerto serie */
				SeriePuts (cadena);
				/* Se suelta el semáforo */
				xSemaphoreGive (sem_serie);
			}else{
				/* Después de 1000 ticks no se ha obtenido el
				semáforo . Se podría dar un aviso o
				simplemente no hacer nada como en este caso 
				o que tooglee un led amarillo */
				Board_LED_Toggle(3); //cambio estado "LED 1" (amarillo)
			}
			entradas_ant = entradas;
		}
	}
}

void RIT_IRQHandler(void)
{
	Board_LED_Toggle(5); //titila "LED 3" ( verde )
	
	portBASE_TYPE xTaskWoken = pdFALSE ;
	
	hora_act .seg ++;
	if( hora_act .seg == 60){
		hora_act .seg = 0;
		hora_act .min ++;
		if( hora_act .min == 60){
			hora_act .min = 0;
			hora_act .hor ++;
			if( hora_act .hor == 24){
				hora_act .hor = 0;
			}
		}
	}
	/* Lanza las tareas */
	xTaskWoken = xSemaphoreGiveFromISR (sem_hora, xTaskWoken);
	
	/* Borra el flag de interrupción */
	Chip_RIT_ClearInt(LPC_RITIMER);

	if( xTaskWoken == pdTRUE ){
		taskYIELD (); /* Si el semáforo ha despertado
						una tarea , se fuerza un cambio
						de contexto */
	}
}

/*==================[external functions definition]==========================*/

int main(void)
{
	initHardware(); /* Inicializa el Hardware del microcontrolador */
	InitTimer();
	InitSerie();
	//InitQueSeYo ();
	/* Se inicializan los semáforos */
	vSemaphoreCreateBinary (sem_serie); //se inicializa por defecto en 1
	vSemaphoreCreateBinary (sem_hora);  //se inicializa por defecto en 1
	xSemaphoreTake (sem_hora , ( portTickType ) 1); //es para que ImprimeHora se bloquee hasta que llegue la 1ra IRQ 
//	xSemaphoreTake (sem_serie , ( portTickType ) 1); // abrazo mortal !
	/* Se crean las tareas */
	xTaskCreate(ImprimeHora, (const char *)"ImpHora", TAM_PILA, NULL, PRIO_IMP_HORA, NULL );
	xTaskCreate(EnviaEntradas, (const char *)"EnvEntr", TAM_PILA, NULL, PRIO_ENV_ENTR, NULL );

	NVIC_EnableIRQ(RITIMER_IRQn); //comentar que hace esta linea .....
	vTaskStartScheduler(); /* y por último se arranca el planificador . */
}

/*==================[end of file]============================================*/
