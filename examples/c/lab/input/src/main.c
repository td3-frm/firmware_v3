#include "stdio.h"

/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"

/* LPCOpen includes*/
#include "board.h"

/* Demo includes. */
#include "supporting_functions.h"

/* Used as a loop counter to create a very crude delay. */
#define mainDELAY_LOOP_COUNT            ( 0xffffff )

/* The task functions. */
void vTask1( void *pvParameters );
void vTask2( void *pvParameters );

/*-----------------------------------------------------------*/

int main( void )
{
   /* Create one of the two tasks. */
   xTaskCreate( vTask1,                   	/* Pointer to the function that implements the task. */
                (const char *)"Task 1",		/* Text name for the task.  This is to facilitate debugging only. */
                configMINIMAL_STACK_SIZE, 	/* Stack depth - most small microcontrollers will use much less stack than this. */
                NULL,                     	/* We are not using the task parameter. */
                tskIDLE_PRIORITY+1,       	/* This task will run at priority 1. */
                NULL );                   	/* We are not using the task handle. */

   /* Create the other task in exactly the same way. */
   xTaskCreate( vTask2, (const char *)"Task 2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL );

   /* Start the scheduler to start the tasks executing. */
   vTaskStartScheduler();

   /* The following line should never be reached because vTaskStartScheduler()
   will only return if there was not enough FreeRTOS heap memory available to
   create the Idle and (if configured) Timer tasks.  Heap management, and
   techniques for trapping heap exhaustion, are described in the book text. */
   for( ;; );
   return 0;
}
/*-----------------------------------------------------------*/

void vTask1( void *pvParameters )
{
   const char *pcTaskName = "Task 1 is running. Switching  LEDs\r\n";
   volatile uint32_t ul;

   /* As per most tasks, this task is implemented in an infinite loop. */
   for( ;; ) {
      /* Print out the name of this task. */
      vPrintString( pcTaskName );

      /* Change led state */
      Board_LED_Set(LED_RED, Board_GPIO_GetStatus(BOARD_GPIO_0));
      Board_LED_Set(LED_1, Board_GPIO_GetStatus(BOARD_GPIO_1)); /* Yellow */
      Board_LED_Set(LED_2, Board_GPIO_GetStatus(BOARD_GPIO_2)); /* Red */
      Board_LED_Set(LED_3, Board_GPIO_GetStatus(BOARD_GPIO_3)); /* Green */


      /* Delay for a period. */
      for( ul = 0; ul < mainDELAY_LOOP_COUNT; ul++ ) {
         /* This loop is just a very crude delay implementation.  There is
         nothing to do in here.  Later exercises will replace this crude
         loop with a proper delay/sleep function. */
      }
   }
}
/*-----------------------------------------------------------*/

void vTask2( void *pvParameters )
{
   const char *pcTaskName = "Task 2 is running\r\n";
   volatile uint32_t ul;

   /* As per most tasks, this task is implemented in an infinite loop. */
   for( ;; ) {
      /* Print out the name of this task. */
      vPrintString( pcTaskName );

      /* Delay for a period. */
      for( ul = 0; ul < mainDELAY_LOOP_COUNT; ul++ ) {
         /* This loop is just a very crude delay implementation.  There is
         nothing to do in here.  Later exercises will replace this crude
         loop with a proper delay/sleep function. */
      }
   }
}


