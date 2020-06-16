/*  Copyright 2020 - FreeRTOS Kernel V10.0.1

 * este codigo NO esta en el libro de Sistemas Empotrados en tiempo real
 * pero el objetivo es intriducir a FreeRTOS usando solo una  tarea que 
 * imprime por el puerto serie, e invocando al planificador
 * algo de manejo de TICKs
    - vTaskDelay()
    - pdMS_TO_TICKS

*/

#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"

/* Demo includes. */
#include "supporting_functions.h"

/* The task function. */

void vTaskFunction( void *pvParameters )
{
   TickType_t xLastWakeTime;
   const TickType_t xDelay500ms = pdMS_TO_TICKS( 500UL );

   //printf ( "tics de delay: %d\r\n",xDelay500ms );
   printf ( "cuantos tics por ms: %d\r\n",portTICK_RATE_MS );

   /* As per most tasks, this task is implemented in an infinite loop. */
   for( ;; ) {
      vPrintString( "Tarea 1\r\n" );
      xLastWakeTime = xTaskGetTickCount();
      printf ( "time %d\r\n",xLastWakeTime );

      vTaskDelay( 500 / portTICK_RATE_MS );
      //vTaskDelay( xDelay500ms );
   }
}

/*-----------------------------------------------------------*/

int main( void )
{
   /* Crea tarea con stack minimo y prioridad idle + 1 ... */
   xTaskCreate( vTaskFunction, "Task 1", configMINIMAL_STACK_SIZE, NULL , tskIDLE_PRIORITY+1, NULL );


	vTaskStartScheduler(); /* y por último se arranca el planificador . */
    //Nunca llegara a ese lazo  .... espero

   for( ;; );
   return 0;
}
/*-----------------------------------------------------------*/


