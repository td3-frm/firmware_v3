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
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*==================[macros and definitions]=================================*/

#define PRIO_IMP_HORA 1
#define TAM_PILA 300

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
//static void initHardware(void);
/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

xSemaphoreHandle sem_hora;
static HORA hora_act ={0,0,0};

/*==================[internal functions definition]==========================*/

static void InitTimer(void)
{
	Chip_RIT_Init(LPC_RITIMER);
	Chip_RIT_SetTimerInterval(LPC_RITIMER,1000);
}

static void ImprimeHora(void )
{
	HORA copia_hora ;
	char cadena [10];
	
	while (1){
		if( xSemaphoreTake (sem_hora , ( portTickType ) 2000 ) == pdTRUE ){
			/* Se bloquea hasta que llegue la interrupción de tiempo */
//			DisableInt();  
			copia_hora = hora_act ;
//			EnableInt ();
			printf (" %02d: %02d: %02d\n", copia_hora.hor, copia_hora.min, copia_hora.seg );
		}
	}
}
void RIT_IRQHandler(void)
//void SysTick_Handler(void)
{
	Board_LED_Toggle(5); //titila "LED 3" ( verde )
	
	portBASE_TYPE xTaskWoken = pdFALSE ;
	
	hora_act.seg ++;
	if( hora_act.seg == 60){
		hora_act.seg = 0;
		hora_act.min ++;
		if( hora_act.min == 60){
			hora_act.min = 0;
			hora_act.hor ++;
			if( hora_act.hor == 24){
				hora_act.hor = 0;
			}
		}
	}
	/* Lanza las tareas */
	xTaskWoken = xSemaphoreGiveFromISR (sem_hora, xTaskWoken);
	/* Borra el flag de interrupción */
	Chip_RIT_ClearInt(LPC_RITIMER);
//	if( xTaskWoken == pdTRUE ){
//		taskYIELD (); /* Si el semáforo ha despertado
//						una tarea , se fuerza un cambio
//						de contexto */
//	}
}




/*==================[external functions definition]==========================*/

int main(void)
{
	//InitQueSeYo ();
	/* Se inicializan los semáforos */
	vSemaphoreCreateBinary (sem_hora);  //se inicializa por defecto en 1
	xSemaphoreTake (sem_hora , ( portTickType ) 1); //es para que ImprimeHora se bloquee hasta que llegue la 1ra IRQ 
	/* Se crean las tareas */
	xTaskCreate(ImprimeHora, (const char *)"ImpHora", TAM_PILA, NULL, PRIO_IMP_HORA, NULL );

	NVIC_EnableIRQ(RITIMER_IRQn); //comentar que hace esta linea .....
	vTaskStartScheduler(); /* y por último se arranca el planificador . */
}

/*==================[end of file]============================================*/
