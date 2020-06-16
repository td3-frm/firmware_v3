/*  Copyright 2020 - FreeRTOS Kernel V10.0.1

    codigo basado en el libro Sistemas Empotrados en tiempo real 
    José Daniel Muñoz Frías
4.7.  Colas para comunicar tareas (Pagina 108)

 Cuando existen varios generadores de datos y un sólo consumidor y
no se desea bloquear a los generadores a la espera de que el consu-
midor obtenga los datos.
4.7.2. (Pagina 111 ) Ejemplo de manejo de colas usando FreeRTOS
En un sistema en tiempo real es necesario guardar un registro de los
errores producidos para que en caso de que se produzca algún problema
con el sistema, los ingenieros puedan al menos tener una idea de qué erro-
res se han producido en el fallo. Las características del sistema serán:
Por cada error se guardará una cadena de caracteres con el siguiente
formato: "Tx:Eyy\n" . La x representa el número de la tarea en la que
se ha producido el error y la y un código que indica el error producido.
Existen 4 tareas en el sistema y cada una podrá enviar registros de
error a la cola
Existe una única tarea que se encarga de leer registros de error de la
cola y escribirlos en una memoria EEPROM. Para escribir datos en la
EEPROM existe la siguiente función:
uint16 EscEEPROM (void *pbuff , uint16 tambuf );
Que escribe en la memoria EEPROM tambuf bytes a partir de la direc-
ción pbuff . La función devuelve el número de bytes que ha escrito en
la memoria EEPROM.

NOTA: Se reemplazo escritura en la EEPROM por puerto serie ;-)

*/
/*==================[inclusions]=============================================*/

#include "board.h"
#include "chip.h"
#include "string.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
//#include "semphr.h"
#include "queue.h"


/*==================[macros and definitions]=================================*/

# define PRIO_T_ERR 1	//menos prioritaria
# define PRIO_T1 2
# define PRIO_T2 3
# define PRIO_T3 4
# define PRIO_T4 5 		//mas prioritaria
# define TAM_PILA 256

# define TAM_COLA 20 /* 20 mensajes */
# define TAM_MSG 8 /* cada mensaje : "Tx:Eyy\n\0" ocupa 8 caracteres */

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/** @brief hardware initialization function
 *	@return none
 */
static void initHardware(void);

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

QueueHandle_t cola_err; // Global, se usa de distintas tareas

/*==================[internal functions definition]==========================*/

static void initHardware(void)
{
    SystemCoreClockUpdate();
    Board_Init();

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

static void InitSerie(void)
{
    //Board_UART_Init (LPC_USART2);
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

static void Tarea1 (void * pvParameters)
{
	char cad_err [8];
	uint8_t entradas, entradas_ant;
	int error_1, error_2 = 0;
    BaseType_t xStatus;

	while (1){
		/* Proceso Tarea1 */
		entradas = LeeEntradas();
		if(entradas & ~entradas_ant & 0x01){
			error_1 = 1;
		}else{
			error_1 = 0;
		}
		entradas_ant = entradas;
		
		if (error_1){
			strcpy (cad_err, "T1:E01\n");
			xStatus = xQueueSend (cola_err, (void *) cad_err, (portTickType) 100);
            if ( xStatus == pdPASS ) Board_LED_Toggle(5); //titila "LED 3" ( verde )

		}
		
		/* Continuación proceso Tarea1 */
		if (error_2){
			strcpy (cad_err, "T1:E02\n");
			xStatus = xQueueSend (cola_err , (void *) cad_err ,(portTickType) 100);
			if ( xStatus == pdPASS ) Board_LED_Toggle(5); //titila "LED 3" ( verde )
		}
		/* Resto proceso Tarea1 */
		vTaskDelay(50 / portTICK_RATE_MS);
	}
}

static void Tarea2 (void * pvParameters)
{
	char cad_err [8];
	uint8_t entradas, entradas_ant;
	int error_1, error_27 = 0;
    BaseType_t xStatus;
	
	while (1){
		/* Proceso Tarea2 */
		entradas = LeeEntradas();
		if(entradas & ~entradas_ant & 0x02){
			error_1 = 1;
		}else{
			error_1 = 0;
		}
		entradas_ant = entradas;

		if (error_1){
			strcpy (cad_err, "T2:E01\n");
			xStatus = xQueueSend (cola_err, (void *) cad_err, (portTickType) 0);
			/* El timeout es 0 para no bloquear la tarea
			si la cola está llena */
			if ( xStatus == pdPASS ) Board_LED_Toggle(4); //titila "LED 2" ( rojo )
		}
		/* Continuación proceso Tarea2 */
		if (error_27){
			strcpy (cad_err , "T2:E27\n");
			xStatus = xQueueSend (cola_err, (void *) cad_err, (portTickType) 0);
			if ( xStatus == pdPASS ) Board_LED_Toggle(4); //titila "LED 2" ( rojo )
		}
		/* Resto proceso Tarea2 */
		vTaskDelay(50 / portTICK_RATE_MS);
	}
}

static void Tarea3 (void * pvParameters)
{
	char cad_err [8];
	uint8_t entradas, entradas_ant;
	int error_10, error_11 = 0;
    BaseType_t xStatus;
	
	while (1){
		/* Proceso Tarea3 */
		entradas = LeeEntradas();
		if(entradas & ~entradas_ant & 0x04){
			error_10 = 1;
		}else{
			error_10 = 0;
		}
		entradas_ant = entradas;

		if (error_10){
			strcpy (cad_err, "T3:E10\n");
			xStatus = xQueueSend (cola_err, (void *) cad_err, (portTickType) 0);
			/* El timeout es 0 para no bloquear la tarea
			si la cola está llena */
			if ( xStatus == pdPASS ) Board_LED_Toggle(3); //titila "LED 1" ( amarillo )
		}
		/* Continuación proceso Tarea3 */
		if (error_11){
			strcpy (cad_err , "T3:E11\n");
			xStatus = xQueueSend (cola_err, (void *) cad_err, (portTickType) 0);
			if ( xStatus == pdPASS ) Board_LED_Toggle(3); //titila "LED 1" ( amarillo )
		}
		/* Resto proceso Tarea3 */
		vTaskDelay(50 / portTICK_RATE_MS);
	}
}

static void Tarea4 (void * pvParameters)
{
	char cad_err [8];
	uint8_t entradas, entradas_ant;
	int error_20, error_21 = 0;
    BaseType_t xStatus;
	
	while (1){
		/* Proceso Tarea4 */
		entradas = LeeEntradas();
		if(entradas & ~entradas_ant & 0x08){
			error_20 = 1;
		}else{
			error_20 = 0;
		}
		entradas_ant = entradas;

		if (error_20){
			strcpy (cad_err, "T4:E24\n");
			xStatus = xQueueSend (cola_err, (void *) cad_err, (portTickType) 0);
			/* El timeout es 0 para no bloquear la tarea
			si la cola está llena */
			if ( xStatus == pdPASS ) Board_LED_Toggle(2); //cambio estado "LED RGB" (azul)
		}
		/* Continuación proceso Tarea4 */
		if (error_21){
			strcpy (cad_err, "T4:E48\n");
			xStatus = xQueueSend (cola_err, (void *) cad_err, (portTickType) 0);
			if ( xStatus == pdPASS ) Board_LED_Toggle(2); //cambio estado "LED RGB" (azul)
		}
		/* Resto proceso Tarea4 */
		vTaskDelay(50 / portTICK_RATE_MS);
	}
}

static void TareaErr (void * pvParameters )
{
//	extern xQueueHandle cola_err ;
	char cad_rec [8];
	char cadena [20];

	while (1){
		if((xQueueReceive (cola_err , (void *) cad_rec,
				(portTickType) 0xFFFFFFFF )) == pdTRUE ){
			/* Se ha recibido un dato. Se escribe en EEPROM */
			//EscEEPROM ((void *) cad_rec, 8);
			/* Se ha recibido un dato. Se escribe en el puerto Serie */
			sprintf (cadena , "Error: %s\n", cad_rec );
			SeriePuts (cadena);
		}
		/* si después de un timeout no se ha recibido nada
		la tarea se vuelve a bloquear a la espera de un
		nuevo dato */
        else Board_LED_Toggle(0); //cambio estado "LED RGB" (rojo)
	}
}


/*==================[external functions definition]==========================*/

int main(void)
{
	initHardware(); /* Inicializa el Hardware del microcontrolador */
	//InitTimer();
	InitSerie();
	//InitQueSeYo ();
	
	/* Se crea la cola 
    La función devuelve un “manejador”(file descriptor, mejor) que ha de pasarse a las funciones
    que envían y reciben datos de la cola creada. */

	cola_err = xQueueCreate (TAM_COLA, TAM_MSG);
	
	/* Se crean las tareas */
	xTaskCreate (TareaErr, (const char *)"TareaE", TAM_PILA, NULL,
				PRIO_T_ERR, NULL);
	xTaskCreate (Tarea1, (const char *)"Tarea1", TAM_PILA, NULL,
				PRIO_T1, NULL);
	xTaskCreate (Tarea2, (const char *)"Tarea2", TAM_PILA , NULL,
				PRIO_T2, NULL);
	xTaskCreate (Tarea3, (const char *)"Tarea3", TAM_PILA , NULL,
				PRIO_T3, NULL );
	xTaskCreate (Tarea4, (const char *)"Tarea4", TAM_PILA , NULL,
				PRIO_T4, NULL );
	
	vTaskStartScheduler(); /* y por último se arranca el planificador . */
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
