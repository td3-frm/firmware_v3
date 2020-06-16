/*  Copyright 2020 - FreeRTOS Kernel V10.0.1

 * este codigo NO esta en el libro de Sistemas Empotrados en tiempo real
 * pero el objetivo es intriducir a FreeRTOS usando dos tareas que 
 * imprimen por el puerto serie, y cambian prioridades

    - vTaskPrioritySet()
    - uxTaskPriorityGet()
*/

/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"
#include "board.h" //definicion de leds

/* Demo includes. */
//#include "supporting_functions.h"


TaskHandle_t xTask2Handle; //variable global guardara id de tarea2

void vTarea1( void *pvParameters )
{
   UBaseType_t uxPriority;
   uxPriority = uxTaskPriorityGet( NULL ); //retorna la propia prioridad

   for( ;; ) {
      printf ( "Task1 esta running\r\n" );

      printf ( "Aumentando la prioridad de Tarea2...\r\n" );
      vTaskPrioritySet( xTask2Handle, ( uxPriority + 1 ) ); // Tarea2 mas prioritaria
      Board_LED_Set(5, FALSE); //apaga "LED 3" ( verde )

   }
}

/*-----------------------------------------------------------*/

void vTarea2( void *pvParameters )
{
   UBaseType_t uxPriority;
   uxPriority = uxTaskPriorityGet( NULL ); //NULL retorna la propia prioridad

   for( ;; ) {
      printf ( "Tarea2 esta running\r\n" );

      printf( "disminuyendo mi prioridad(Tarea2)\r\n" );
      vTaskPrioritySet( NULL, ( uxPriority - 2 ) ); //NULL propio ID
      Board_LED_Set(5, TRUE); //prende "LED 3" ( verde )
   }
}
/*-----------------------------------------------------------*/

int main( void )
{
   xTaskCreate( vTarea1, "Tarea1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, NULL );
   xTaskCreate( vTarea2, "Tarea2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, &xTask2Handle );

	vTaskStartScheduler(); /* y por último se arranca el planificador . */
    //Nunca llegara a ese lazo  .... espero
     for( ;; );
     return 0;

}
