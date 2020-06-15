/* Copyright 2017
4.9.  Gestión de tiempo
4.9.2.  Ejemplo: arranque de una bomba (Pagina 124)
La función ArrancaBomba arranca una
bomba mediante un arrancador estrella/triángulo y después de un tiempo
monitoriza la presión de salida de la bomba para verificar que ésta está
funcionando correctamente. El tiempo de retardo desde que se conecta el
motor en triángulo hasta que se conecta en estrella es de 500 ms, mientras
que el tiempo que se espera antes de verificar si se está bombeando es igual
a 1 minuto, por si acaso la bomba estaba descargada.
	usa funcion vTaskDelay()
 + 
4.9.3.  Ejemplo: tarea periódica (Pagina 125)
	usa funcion vTaskDelayUntil()
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

#define PRIO_ARRANQUE 2 
#define PRIO_TAR_PER 3	//mayor prioridad para tarea periodica ... para que no cambie el periodo ..
#define TAM_PILA 1024

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void InitHardware(void)
{
    SystemCoreClockUpdate();
    Board_Init();
}

//uint8_t LeeEntradas(void)
//{
//	return Buttons_GetStatus();
//}

void ConectaTensionEstrella(void)
{
	Board_LED_Set(3, TRUE); //prende "LED 1" (amarillo)
}

void ConectaTensionTriangulo(void)
{
	Board_LED_Set(3, FALSE); //apaga "LED 1" (amarillo)
	Board_LED_Set(5, TRUE); //enciende "LED 3" (verde)
}

void DesconectaTension(void)
{
	Board_LED_Set(5, FALSE); //apaga "LED 3" (verde)
}

void AlarmaFalloArranque(void)
{
	Board_LED_Set(4, TRUE); //enciende "LED 2" (rojo)
	while (1); /* Se queda bloqueado el sistema hasta que
				venga el técnico de mantenimiento */
}

uint8_t PresionOK(void)
{
	return (Buttons_GetStatus() & 0x01); //solo si se esta apretando la "TEC 1", retorna != 0 
}

void ArrancaBomba(void)
{
	ConectaTensionEstrella();
	vTaskDelay (1000/portTICK_RATE_MS); /*1000 ms ... o 1 seg */
	ConectaTensionTriangulo();
	vTaskDelay (5000/portTICK_RATE_MS); /*5 seg */
	if( PresionOK()==0){ /* No hay presión . Por tanto la
							bomba no está funcionando */
		DesconectaTension();
		AlarmaFalloArranque();
	}
}

void Arranque(void *pvParameters)
{
	while(1){
		ArrancaBomba();
		vTaskDelay (10000/portTICK_RATE_MS); // espera 10 segundos mas y reinicia todo
		Board_LED_Set(5, FALSE); // apaga  "LED 3" (verde)
	}
}

void TareaPeriodica (void *pvParameters)
{
	portTickType xLastWakeTime;
	portTickType xPeriodo;
	uint8_t valor;
	
	xPeriodo = 500/portTICK_RATE_MS ; /* Periodo 500 ms */
	
	/* Inicializa xLastWakeTime con el tiempo actual */
	xLastWakeTime = xTaskGetTickCount();
	while (1){
		vTaskDelayUntil (&xLastWakeTime, xPeriodo ); /* Espera
										el siguiente periodo */
		/* Realiza su proceso */
		Chip_GPIO_SetPortValue(LPC_GPIO_PORT, 5, valor); /* cambia valor de RGB ? */
		valor++;	
	}
}

/*==================[external functions definition]==========================*/

int main(void)
{
	InitHardware(); /* Inicializa el Hardware del microcontrolador */

	/* Se crean las tareas */
	xTaskCreate(Arranque, (const char *)"Arranque", TAM_PILA, NULL, PRIO_ARRANQUE, NULL );
//	xTaskCreate(TareaPeriodica, (const char *)"TareaPerio", TAM_PILA, NULL, PRIO_TAR_PER, NULL );
	
	vTaskStartScheduler(); /* y por último se arranca el planificador . */
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
