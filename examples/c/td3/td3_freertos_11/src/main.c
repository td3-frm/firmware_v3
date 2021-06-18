/*  Copyright 2020 - FreeRTOS Kernel V10.0.1

    codigo NO basado en el libro Sistemas Empotrados en tiempo real 
    José Daniel Muñoz Frías
4.5.6.  Abrazo mortal (pagina 105)

Otro problema que puede ocurrir cuando se usan semáforos se ilustra a continuación:
Dos tareas y dos semáforos.
Supóngase que inicialmente ambos semáforos están libres. Supóngase también que
se está ejecutando la Tarea1 y que pide el semáforo p_sem_a . Como está
libre, la llamada a xSemaphoreTake retornará inmediatamente. Supóngase ahora
que antes de que a la Tarea1 le de tiempo a realizar la segunda llamada
a xSemaphoreTake se produce un cambio de contexto. Supóngase que este cam-
bio de contexto hace que comience a ejecutarse la Tarea2. Ésta pedirá en
primer lugar el semáforo p_sem_b , el cual como está libre se le dará. A con-
tinuación pedirá el semáforo p_sem_a , pero como está cogido por la Tarea1,
la Tarea2 se bloqueará a la espera de que dicho semáforo quede libre (la
llamada a xSemaphoreTake no retornará). Se producirá ahora un nuevo cambio
de contexto, pues la Tarea2 no puede continuar. Cuando se ejecute la Ta-
rea1, fruto de este cambio de contexto, ésta continuará la ejecución donde
la dejó, y pedirá el semáforo p_sem_b . Como este semáforo está cogido por
la Tarea2, la Tarea1 volverá a bloquearse. Como la Tarea1 está esperando
el semáforo p_sem_b que sólo puede soltar la Tarea2 y la Tarea2 está es-
perando el semáforo p_sem_a que sólo puede soltar la Tarea1, ninguna de
las tareas podrá ejecutarse más. Esto es lo que se conoce como un abrazo
mortal (Deadlock en inglés).

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

SemaphoreHandle_t p_sem_a;
SemaphoreHandle_t p_sem_b;

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void vTarea2(void *pvParameters)
{
  const char *pcTaskName = "Tarea 2 mas prioritaria";
  const TickType_t xDelay50ms = pdMS_TO_TICKS( 60UL );

   for( ;; ) {
      xSemaphoreTake( p_sem_a, portMAX_DELAY);
      Board_LED_Set(3,TRUE); //Prende "LED 3" ( amarillo )
      printf( "%s Toma sem_a \r\n", pcTaskName );
      vTaskDelay( xDelay50ms ); //para que SIEMPRE haya Deadlock ;-)
      xSemaphoreTake( p_sem_b, portMAX_DELAY);
      Board_LED_Set(5,TRUE); //Enciende "LED 5" ( verde )
      printf( "%s Toma sem_b \r\n", pcTaskName );
      /* El puerto serie está libre */
      xSemaphoreGive(p_sem_a);
      Board_LED_Set(3,FALSE); //Apaga "LED 3" ( amarillo )
      xSemaphoreGive(p_sem_b);
      Board_LED_Set(5,FALSE); //Apaga "LED 5" ( verde )
   }
}

static void vTarea1(void *pvParameters)
{
   const char *pcTaskName = "Tarea 1 menos prioritaria ";
   const TickType_t xDelay50ms = pdMS_TO_TICKS( 50UL );
   const TickType_t xDelay200ms = pdMS_TO_TICKS( 200UL );

   for( ;; ) {
      xSemaphoreTake( p_sem_b, portMAX_DELAY);
      Board_LED_Set(5,TRUE); //Prende "LED 5" ( verde )
      printf( "%s Toma sem_b \r\n", pcTaskName );
      xSemaphoreTake( p_sem_a, portMAX_DELAY);
      //if ((xSemaphoreTake( p_sem_a, xDelay200ms)) == pdPASS) {
         // Board_LED_Set(3,TRUE); //Prende "LED 3" ( amarillo )
          printf( "%s Toma sem_a \r\n", pcTaskName );
          /* El puerto serie está libre */
          xSemaphoreGive(p_sem_b);
          Board_LED_Set(5,FALSE); //Apaga "LED 5" ( verde )
          xSemaphoreGive(p_sem_a);
          Board_LED_Set(3,FALSE); //Apaga "LED 3" ( amarillo )
     /* }
      else { 
        xSemaphoreGive(p_sem_b);
        Board_LED_Set(5,FALSE); //Apaga "LED 5" ( verde )
        printf( "Sale temporalmente de interbloqueo por timeout ... \r\n" );
        vTaskDelay( xDelay50ms ); 
        }*/ 

   }
}

/*==================[external functions definition]==========================*/

int main(void)
{
    //Se inicializa HW
	/* Se crean las tareas */
    printf("iniciando ...\r\n");
	p_sem_a = xSemaphoreCreateMutex ();  //se inicializa por defecto en 1
	p_sem_b = xSemaphoreCreateMutex ();  //se inicializa por defecto en 1

	xTaskCreate(vTarea1, (const char *)"Tarea1", TAM_PILA, NULL, PRIO_TAREA1, NULL );
	xTaskCreate(vTarea2, (const char *)"Tarea2", TAM_PILA, NULL, PRIO_TAREA2, NULL );

	vTaskStartScheduler(); /* y por último se arranca el planificador . */
    //Nunca llegara a ese lazo  .... espero
     for( ;; );
     return 0;

}

/*==================[end of file]============================================*/
