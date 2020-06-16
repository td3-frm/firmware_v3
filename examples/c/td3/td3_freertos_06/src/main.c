/*  Copyright 2020 - FreeRTOS Kernel V10.0.1

	codigo basado en el libro Sistemas Empotrados en tiempo real 
    José Daniel Muñoz Frías
4.5.1. Ejemplo (Pagina 96) (Ejemplo usando FreeRTOS) 

Para ilustrar el funcionamiento de los semáforos, supóngase que en un
sistema se necesita enviar la hora constantemente (en realidad sólo cuando
cambie) por el puerto serie y el estado de 8 entradas digitales. Por tanto,
el puerto serie se comparte ahora por dos tareas: ImprimeHora() para imprimir
la hora y EnviaEntradas() para imprimir el estado de las entradas.
Será por tanto necesario arbitrar el acceso al puerto serie por ambas tareas,
por ejemplo mediante un semáforo (sem_serie)
sem_serie 	->	exclusion mutua para seccion critica

nota: en minicom agregar Add Carriage Ret -> ctr+a z u
      velocidad en 115200 -> ctr+a z o
 */

/*==================[inclusions]=============================================*/

#include "board.h"
#include "chip.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "semphr.h"
/* Demo includes. */
#include "supporting_functions.h"


/*==================[macros and definitions]=================================*/

#define PRIO_IMP_HORA 2
#define PRIO_ENV_ENTR 1

typedef struct {
uint8_t hor;
uint8_t min;
uint8_t seg;
}HORA;

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/** @brief hardware initialization function
 *	@return none
 */
//static void initHardware(void);
static void initHardware(void)
{
    SystemCoreClockUpdate();
    Board_Init();
 
}

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

SemaphoreHandle_t sem_serie;
static HORA hora_act ={0,0,0};

/*==================[internal functions definition]==========================*/


static void InitSerie(void)
{
    Chip_UART_Init(LPC_USART2);
	Chip_UART_SetBaud(LPC_USART2, 115200);  /* Set Baud rate */
	Chip_UART_ConfigData(LPC_USART2, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT));
	Chip_UART_SetupFIFOS(LPC_USART2, UART_FCR_FIFO_EN | UART_FCR_TRG_LEV3); /* Modify FCR (FIFO Control Register)*/
	Chip_UART_TXEnable(LPC_USART2); /* Enable UART Transmission */
	Chip_SCU_PinMux(7, 1, MD_PDN, FUNC6);              /* P7_1,FUNC6: UART2_TXD */
	Chip_SCU_PinMux(7, 2, MD_PLN|MD_EZI|MD_ZI, FUNC6); /* P7_2,FUNC6: UART2_RXD */
}

static void SeriePuts(char *data)
{
	while(*data != 0)
	{
		while ((Chip_UART_ReadLineStatus(LPC_USART2) & UART_LSR_THRE) == 0) {}
		Chip_UART_SendByte(LPC_USART2, *data);
		data++;
	}
}

static void InitTimer(void)
{
    Chip_RIT_Init(LPC_RITIMER);
    Chip_RIT_SetTimerInterval(LPC_RITIMER,1000);
}

void RIT_IRQHandler(void) //ISR tarea background
{
	Board_LED_Toggle(5); //titila "LED 3" ( verde )
	
	hora_act.seg++;
	if( hora_act.seg == 60){
		hora_act.seg = 0;
		hora_act.min++;
		if( hora_act.min == 60){
			hora_act.min = 0;
			hora_act.hor++;
			if( hora_act.hor == 24){
				hora_act.hor = 0;
			}
		}
	}
    Chip_RIT_ClearInt(LPC_RITIMER);
}

static uint8_t LeeEntradas(void)
{
    uint8_t tecla = 0;
    if (Board_TEC_GetStatus(BOARD_TEC_1) == 0) tecla = tecla + 1;
    if (Board_TEC_GetStatus(BOARD_TEC_2) == 0) tecla = tecla + 2;
    if (Board_TEC_GetStatus(BOARD_TEC_3) == 0) tecla = tecla + 4;
    if (Board_TEC_GetStatus(BOARD_TEC_4) == 0) tecla = tecla + 8;

    return tecla;
}

static void ImprimeHora(void * a)
{
	HORA copia_hora ;
	char cadena [10];
    const TickType_t xDelay1000ms = pdMS_TO_TICKS( 1000UL ); //macro para convertir ms en ticks

	while (1){
			/* Se bloquea hasta que llegue la interrupción de tiempo */
            vTaskDelay( xDelay1000ms );  // tarea pasa a estado Bloqueado hasta que expira timer
            
            //__disable_irq(); // deshabilita interrupciones
			copia_hora = hora_act;
            //__enable_irq(); // habilita interrupciones

			sprintf (cadena , " %02d: %02d: %02d\n", copia_hora.hor, copia_hora.min, copia_hora.seg );

            if (( xSemaphoreTake( sem_serie, (portTickType) 1000 )) == pdTRUE) {
			    SeriePuts (cadena); /* Se tiene el semáforo : se puede
                                        acceder al puerto serie */
                //printf(cadena);
				xSemaphoreGive (sem_serie);
		}
        else{
            /*  Después de 1000 ticks no se ha obtenido el
                semáforo . Se podría dar un aviso o
                simplemente no hacer nada como en este caso.... mejor avisamos con el red ROJO */
	            Board_LED_Toggle(4); //titila "LED 2" ( rojo )
            }
	}
}

static void EnviaEntradas(void * a)
{
	char cadena [100]; /* Guarda el mensaje a transmitir */
	uint8_t entradas;
	static uint8_t entradas_ant = 0;

	while (1){
		entradas = LeeEntradas();
		if( entradas_ant != entradas ){ /* Sólo imprime si cambian las entradas */
			sprintf (cadena , "Entradas: %x\n", entradas );
			if( xSemaphoreTake (sem_serie , ( portTickType ) 1000) == pdTRUE ){
				/* Se tiene el semáforo : se puede acceder al puerto serie */
				SeriePuts (cadena);
                //printf(cadena);
				/* Se suelta el semáforo */
				xSemaphoreGive (sem_serie);
			}else{
				/* Después de 1000 ticks no se ha obtenido el
				semáforo . Se podría dar un aviso o
				simplemente no hacer nada como en este caso ..mejor avisamos con el led amarillo */
				Board_LED_Toggle(3); //cambio estado "LED 1" (amarillo)
			}
			entradas_ant = entradas;
		}
	}
}


/*==================[external functions definition]==========================*/

int main(void)
{
	initHardware(); /* Inicializa el Hardware del microcontrolador */
	InitTimer();
	InitSerie();
	//InitQueSeYo();
	/* Se inicializa el semáforo */
	sem_serie = xSemaphoreCreateBinary ();  //se inicializa por defecto en 0
	xSemaphoreGive (sem_serie); //caso contrario ninguna tarea trabaja
	/* Se crean las tareas */
	xTaskCreate(ImprimeHora , (const char *)"ImpHora", 512, NULL, PRIO_IMP_HORA, NULL );
    xTaskCreate(EnviaEntradas, (const char *)"EnvEntr", 512, NULL, PRIO_ENV_ENTR, NULL );

    NVIC_EnableIRQ(RITIMER_IRQn);

	vTaskStartScheduler(); /* y por último se arranca el planificador . */
    for( ;; );
    return 0;
}

/*==================[end of file]============================================*/

