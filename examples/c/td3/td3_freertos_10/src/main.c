/*  Copyright 2020 - FreeRTOS Kernel V10.0.1

    codigo NO basado en el libro Sistemas Empotrados en tiempo real 
    José Daniel Muñoz Frías
 
4.5.5. Inversión de prioridad (pagina 104)
El uso de semáforos puede producir una inversión de prioridad, 
que consiste en que una tarea de menor prioridad impide
la ejecución de una tarea de mayor prioridad.

 *  */
 /*==================[inclusions]=============================================*/

#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*==================[macros and definitions]=================================*/

#define PRIO_TAREA1 1
#define PRIO_TAREA2 2
#define TAM_PILA 150
#define mainDELAY_LOOP_COUNT        ( 0xffffff )

SemaphoreHandle_t sem_exclu;

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void vTarea1(void *pvParameters)
{
  const char *pcTaskName = "Tarea 1 menos prioritaria\r\n";
   uint32_t ul;
   uint32_t ul2;

   for( ;; ) {
      xSemaphoreTake( sem_exclu, portMAX_DELAY);
      /* El puerto serie está libre */
      Board_LED_Set(3,TRUE); //Enciende "LED 1" ( amarillo )
      printf( pcTaskName );
      /* Delay for a period.  x  8 */
      for( ul = 0; ul < mainDELAY_LOOP_COUNT; ul++ ) {
          for( ul2 = 0; ul2 < 8; ul2++ ) {
          }
      }
      xSemaphoreGive(sem_exclu);
      Board_LED_Set(3,FALSE); //Apaga "LED 1" ( amarillo )
   }
}

static void vTarea2(void *pvParameters)
{
   const char *pcTaskName = "Tarea 2 mas prioritaria \r\n";
   const TickType_t xDelay200ms = pdMS_TO_TICKS( 200UL );

   for( ;; ) {
      /* pasa a READY cada 200ms. */
      vTaskDelay( xDelay200ms );
      xSemaphoreTake( sem_exclu, portMAX_DELAY);
      Board_LED_Set(5,TRUE); //Enciende "LED 3" ( verde )
      /* El puerto serie está libre */
      printf( pcTaskName );
      xSemaphoreGive(sem_exclu);
      Board_LED_Set(5,FALSE); //Apaga "LED 3" ( verde )
   }
}

/*==================[external functions definition]==========================*/

int main(void)
{
    //Se inicializa HW
	/* Se crean las tareas */
	sem_exclu = xSemaphoreCreateBinary ();  //se inicializa por defecto en 0
//	sem_exclu = xSemaphoreCreateMutex ();  //se inicializa por defecto en 1
    xSemaphoreGive (sem_exclu); //caso contrario ninguna tarea trabaja

	xTaskCreate(vTarea1, (const char *)"Tarea1", TAM_PILA, NULL, PRIO_TAREA1, NULL );
	xTaskCreate(vTarea2, (const char *)"Tarea2", TAM_PILA, NULL, PRIO_TAREA2, NULL );

	vTaskStartScheduler(); /* y por último se arranca el planificador . */
    //Nunca llegara a ese lazo  .... espero
     for( ;; );
     return 0;

}

/*==================[end of file]============================================*/
