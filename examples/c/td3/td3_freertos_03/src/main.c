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
const char *pcTextoTarea1 = "Tarea1 is running\r\n";
const char *pcTextoTarea2 = "Tarea2 is running\r\n";

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void vTarea(void *pvParameters)
{

   const TickType_t xDelay500ms = pdMS_TO_TICKS( 500UL ); //macro para convertir ms en ticks
   char *pcTaskName;
   /* The string to print out is passed in via the parameter.  Cast this to a
   character pointer. */
   pcTaskName = ( char * ) pvParameters;
   for ( ;; ){
       printf( pcTaskName );
       if ( pcTaskName == "Tarea1 is running\r\n" ) Board_LED_Toggle(3); //titila "LED 1" ( amarillo )
       else Board_LED_Toggle(5); //titila "LED 3" ( verde )
       //vTaskDelay(en_ticks)
       //vTaskDelay( xDelay500ms );  // tarea pasa a estado Bloqueado hasta que expira timer
       vTaskDelay( 1000 / portTICK_RATE_MS);  // tarea pasa a estado Bloqueado hasta que expira timer
       //configTICK_RATE_HZ = 100 en el ../include , 1 tick cada 10 ms
       //#define portTICK_RATE_MS          ( ( TickType_t ) 1000 / configTICK_RATE_HZ )

   }
}


/*==================[external functions definition]==========================*/

int main(void)
{
    //Se inicializa HW
	/* Se crean las tareas */
	xTaskCreate(vTarea, (const char *)"Tarea1", TAM_PILA, (void*)pcTextoTarea1, tskIDLE_PRIORITY, NULL );
	xTaskCreate(vTarea, (const char *)"Tarea2", TAM_PILA, (void*)pcTextoTarea2, tskIDLE_PRIORITY+2, NULL );

	vTaskStartScheduler(); /* y por último se arranca el planificador . */
    //Nunca llegara a ese lazo  .... espero
     for( ;; );
     return 0;
}

/*==================[end of file]============================================*/
