/*  Copyright 2020 - FreeRTOS Kernel V10.0.1

 * este codigo NO esta en el libro de Sistemas Empotrados en tiempo real
 * pero el objetivo es intriducir a FreeRTOS usando dos tareas que 
 * imprime por el puerto serie, e invocando al planificador

    -  vTaskDelayUntil()
    -  xTaskGetTickCount()  --- solo una vez .. despues se encarga el operativo

*/

/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "board.h" //definicion de leds


/* Demo includes. */
#include "supporting_functions.h"


const char *pcTextForTask1 = "Continuous task 1 running\r\n";
const char *pcTextForTask2 = "Continuous task 2 running\r\n";
const char *pcTextForPeriodicTask = "Periodic task is running\r\n";

void vContinuousProcessingTask( void *pvParameters )
{
   char *pcTaskName;
   //pasado el nombre por parametro
   pcTaskName = ( char * ) pvParameters; //casteo de void a char

   for( ;; ) {
      vPrintString( pcTaskName ); 
      //printf ( pcTaskName ); 
   }
}
/*-----------------------------------------------------------*/

void vPeriodicTask( void *pvParameters )
{
   TickType_t xLastWakeTime;
   const TickType_t xDelay50ms = pdMS_TO_TICKS( 50UL );

   xLastWakeTime = xTaskGetTickCount(); /* se inicializa la variable con la 
   actual cantidad de ticks. Luego es manejada por la API de vTaskDelayUntil()*/

   for( ;; ) {
      vPrintString( "Periodic task is running\r\n" );
      Board_LED_Toggle(4); //titila "LED 2" ( rojo )

      /* pasa a READY cada 50ms. */
      vTaskDelayUntil( &xLastWakeTime, xDelay50ms );
   }
}

/*-----------------------------------------------------------*/

int main( void )
{
   xTaskCreate( vContinuousProcessingTask, "Task 1", configMINIMAL_STACK_SIZE, (void*)pcTextForTask1, tskIDLE_PRIORITY+1, NULL );
   xTaskCreate( vContinuousProcessingTask, "Task 2", configMINIMAL_STACK_SIZE, (void*)pcTextForTask2, tskIDLE_PRIORITY+1, NULL );

   xTaskCreate( vPeriodicTask, "Task 3", configMINIMAL_STACK_SIZE*2, (void*)pcTextForPeriodicTask, tskIDLE_PRIORITY+1, NULL );

	vTaskStartScheduler(); /* y por último se arranca el planificador . */
    //Nunca llegara a ese lazo  .... espero

   for( ;; );
   return 0;
}
/*-----------------------------------------------------------*/
