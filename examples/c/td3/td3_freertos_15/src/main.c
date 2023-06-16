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

#define TAM_PILA 150
#define mainDELAY_LOOP_COUNT        ( 0xffffff )

SemaphoreHandle_t sem_exclu;

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void vTareaL(void *pvParameters)
{
  const char *pcTaskName = "Tarea L menos prioritaria\r\n";
   uint32_t ul;
   uint32_t ul2;

   for( ;; ) {
      xSemaphoreTake( sem_exclu, portMAX_DELAY);
      /* El puerto serie está libre */
      Board_LED_Set(3,TRUE); //Enciende "LED 1" ( amarillo )
      printf( pcTaskName );
      /* Delay for a period.  x  8 */
      for( ul = 0; ul < mainDELAY_LOOP_COUNT; ul++ ) {
          for( ul2 = 0; ul2 < 10; ul2++ ) {
          }
      }
      xSemaphoreGive(sem_exclu);
      Board_LED_Set(3,FALSE); //Apaga "LED 1" ( amarillo )
   }
}

static void vTareaH(void *pvParameters)
{
   const char *pcTaskName = "Tarea H mas prioritaria \r\n";
   const TickType_t xDelay1000ms = pdMS_TO_TICKS( 1000UL );

   for( ;; ) {
      /* pasa a READY cada 200ms. */
      vTaskDelay( xDelay1000ms );
      xSemaphoreTake( sem_exclu, portMAX_DELAY);
      Board_LED_Set(5,TRUE); //Enciende "LED 3" ( verde )
      /* El puerto serie está libre */
      printf( pcTaskName );
      xSemaphoreGive(sem_exclu);
      Board_LED_Set(5,FALSE); //Apaga "LED 3" ( verde )
   }
}

static void vTareaM(void *pvParameters)
{
  const char *pcTaskName = "Tarea M mediana prioridad\r\n";
   uint32_t ul;
   //volatile uint32_t ul;
   const TickType_t xDelay10ms = pdMS_TO_TICKS( 10UL );
   vTaskDelay( xDelay10ms );

   /* As per most tasks, this task is implemented in an infinite loop. */
   for( ;; ) {
      /* Print out the name of this task. */
      printf( pcTaskName );
      Board_LED_Toggle(4); //titila "LED 2" ( rojo )
      /* Delay for a period. */
      for( ul = 0; ul < mainDELAY_LOOP_COUNT; ul++ ) {
      }
   }
}

/*==================[external functions definition]==========================*/

int main(void)
{
    //Se inicializa HW
	/* Se crean las tareas */
//	sem_exclu = xSemaphoreCreateBinary ();  //se inicializa por defecto en 0
	sem_exclu = xSemaphoreCreateMutex ();  //se inicializa por defecto en 1
 //   xSemaphoreGive (sem_exclu); //caso contrario ninguna tarea trabaja

	xTaskCreate(vTareaL, (const char *)"TareaL", TAM_PILA, NULL, tskIDLE_PRIORITY+1 , NULL );
	xTaskCreate(vTareaM, (const char *)"Tarea1", TAM_PILA, NULL, tskIDLE_PRIORITY+2 , NULL );
	xTaskCreate(vTareaH, (const char *)"TareaH", TAM_PILA, NULL, tskIDLE_PRIORITY+3 , NULL );

	vTaskStartScheduler(); /* y por último se arranca el planificador . */
    //Nunca llegara a ese lazo  .... espero
     for( ;; );
     return 0;

}

/*==================[end of file]============================================*/
