/* este codigo NO esta en el libro de Sistemas Empotrados en tiempo real
 * pero el objetivo es intriducir a FreeRTOS usando solo una  tarea que 
 * imprime por el puerto serie, e invocando al planificador
 *
 *  - xTaskCreate()
 *  - vTaskStartScheduler()
 *
 * esto hará un busy waiting ... :-(
 *
 *  */
 /*==================[inclusions]=============================================*/

#include "board.h"
#include "FreeRTOS.h"
#include "task.h"

/*==================[macros and definitions]=================================*/

#define PRIO_TAREA1 1
#define PRIO_TAREA2 1
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
       //vTaskDelay(en_ticks)
       vTaskDelay( xDelay500ms );  // tarea pasa a estado Bloqueado hasta que expira timer
       //vTaskDelay( 1000 / portTICK_RATE_MS);  // tarea pasa a estado Bloqueado hasta que expira timer
   }
}


/*==================[external functions definition]==========================*/

int main(void)
{
    //Se inicializa HW
	/* Se crean las tareas */
	xTaskCreate(vTarea, (const char *)"Tarea1", TAM_PILA, (void*)pcTextoTarea1, tskIDLE_PRIORITY+1, NULL );
	xTaskCreate(vTarea, (const char *)"Tarea2", TAM_PILA, (void*)pcTextoTarea2, tskIDLE_PRIORITY+1, NULL );

	vTaskStartScheduler(); /* y por último se arranca el planificador . */
    //Nunca llegara a ese lazo  .... espero
     for( ;; );
     return 0;
}

/*==================[end of file]============================================*/
