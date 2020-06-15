/* Copyright 2017
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

uint8_t LeeEntradas(void)
{
	return Buttons_GetStatus();
}

void ConectaTensionEstrella(void)
{
	Board_LED_Set(4, TRUE);
}

void ConectaTensionTriangulo(void)
{
	Board_LED_Set(5, TRUE);
}

void DesconectaTension(void)
{
	Board_LED_Set(4, FALSE);
	Board_LED_Set(5, FALSE);
}

void AlarmaFalloArranque(void)
{
	Board_LED_Set(3, TRUE);
}

uint8_t PresionOK(void)
{
	return (Buttons_GetStatus() & 0x01);
}

void ArrancaBomba(void)
{
	ConectaTensionEstrella();
	vTaskDelay (1000/portTICK_RATE_MS);
	ConectaTensionTriangulo();
	vTaskDelay (10000/portTICK_RATE_MS);
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
		vTaskDelay (1000/portTICK_RATE_MS);
		Board_LED_Set(3, FALSE);
	}
}


/*==================[external functions definition]==========================*/

int main(void)
{
	InitHardware(); /* Inicializa el Hardware del microcontrolador */

	/* Se crean las tareas */
	xTaskCreate(Arranque, (const char *)"Arranque", TAM_PILA, NULL, PRIO_ARRANQUE, NULL );

	vTaskStartScheduler(); /* y por último se arranca el planificador . */
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
