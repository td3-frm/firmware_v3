/*  Copyright 2020 - FreeRTOS Kernel V10.0.1

 * este codigo NO esta en el libro de Sistemas Empotrados en tiempo real
 * pero el objetivo es intriducir a FreeRTOS usando solo una (o dos) tarea que 
 * imprime por el puerto serie, e invocando al planificador
 *
 *  - xTaskCreate()
 *  - vTaskStartScheduler()
 *  - vTaskDelay(ticks)
 *
 *  */
 /*==================[inclusions]=============================================*/

#include "board.h"
#include "FreeRTOS.h"
#include "task.h"

/*==================[macros and definitions]=================================*/

#define TAM_PILA 150

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void vTarea(void *pvParameters)
{
   uint8_t seg = 0;
   uint8_t min = 0;
   uint8_t hor = 0;

   const TickType_t xDelay1000ms = pdMS_TO_TICKS( 1000UL ); //macro para convertir ms en ticks
   //probar luego aumentando el configTICK_RATE_HZ
   TickType_t xLastWakeTime = xTaskGetTickCount(); 

   for ( ;; ){
       //vTaskDelay( xDelay1000ms );  
       vTaskDelayUntil( &xLastWakeTime, xDelay1000ms );
       seg++;
       Board_LED_Toggle(5);
       printf (" %02d: %02d: %02d  nro quantum: %d \r\n", hor, min, seg, xTaskGetTickCount() );
       if(seg == 60){
           seg = 0;
           min ++;
           if(min == 60){
               min = 0;
               hor ++;
               if(hor == 24){
                   hor = 0;
              }
           }
       }
   }
}


/*==================[external functions definition]==========================*/

int main(void)
{
    //Se inicializa HW
	/* Se crean las tareas */
	xTaskCreate(vTarea, (const char *)"Tarea1", TAM_PILA, NULL, tskIDLE_PRIORITY+1, NULL );

	vTaskStartScheduler(); /* y por último se arranca el planificador . */
    //Nunca llegara a ese lazo  .... espero
     for( ;; );
     return 0;
}

/*==================[end of file]============================================*/
