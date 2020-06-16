/*  Copyright 2020 
        codigo basado en el libro Sistemas Empotrados en tiempo real 
        José Daniel Muñoz Frías
1.6.2.  Datos compartidos (Pagina 21)

Al trasladar los procesos lentos a la tarea de primer plano se soluciona
el problema de la latencia, pero por desgracia se crea un nuevo problema:
la comunicación entre tareas que se ejecutan asíncronamente.
En este ejemplo las dos tareas se comunican compartiendo tres varia-
bles globales. El problema que se origina se denomina incoherencia de
datos y se ilustra con el siguiente ejemplo: supóngase que en el programa
anterior la interrupción del temporizador se produce cuando se están co-
piando los argumentos de la tarea ImprimeHora , en concreto cuando se ha
copiado el minuto de la hora actual, pero aún no se ha copiado el segundo
Además, como las leyes de Murphy se cumplen siempre, se puede esperar 
que en esta interrupción la variable ms llegue a 1000, con lo cual se 
actualizarán los segundos. Si además  la variable seg es igual a 59,
dicha variable pasará a valer 0 y la variable min se incrementará en 1.
En ese momento la interrupción terminará, devolviendo el control a la tarea
de primer plano que copiará el valor actualizado de seg antes de llamar a
la tarea ImprimeHora . Lo que se vería en la pantalla es lo siguiente, supo-
niendo que la interrupción se ha producido en la llamada que imprime la
segunda línea:
13:13:59
13:13:00
13:14:00
nota: en minicom velocidad en 300 bauds -> ctr+a z o

se mejora copiando la estructura HORA a una estructura local (copia_hora) para la funcion ImprimeHora
para tratar de evitar la "incoherencia" ... pero solo la mejora
se cambiaron de nombre las funciones para que se corresponda con td3_freertos_-1

De acuerdo a la hoja 22,
El problema que se acaba de exponer se origina al producirse una inte-
rrupción dentro de lo que se denomina una zona crítica. Por zona crítica
se entiende toda zona de código en la que se usan recursos compartidos
entre dos tareas que se ejecutan de forma asíncrona, como por ejemplo en-
tre una tarea de primer plano y una de segundo plano, o dos de segundo
plano.
Para evitar incoherencias de datos es necesario conseguir que la eje-
cución de la zona crítica se realice de principio a fin sin ningún tipo de
interrupción. Por tanto una solución podría ser INHABILITAR las interrupcio-
nes durante la ejecución de la zona crítica:
 */

/*==================[inclusions]=============================================*/

#include "board.h"
#include "main.h"

/*==================[macros and definitions]=================================*/

typedef struct {
uint8_t hor;
uint8_t min;
uint8_t seg;
uint16_t useg;
}HORA;

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/


/*==================[external data definition]===============================*/

static HORA hora_act ={0,0,0,0};
/*==================[internal functions definition]==========================*/

static void initHardware(void)
{
    SystemCoreClockUpdate();
    Board_Init();
}

static void InitSerie(void)
{
    Chip_UART_Init(LPC_USART2);
	Chip_UART_SetBaud(LPC_USART2, 300);  /* Set Baud rate */
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
	Chip_RIT_SetTimerInterval(LPC_RITIMER,1);
}

static void ImprimeHora(void)
{
	HORA copia_hora;
	char cadena [10];
	__disable_irq(); // deshabilita interrupciones 
	copia_hora = hora_act;
	__enable_irq(); // habilita interrupciones 
	sprintf (cadena , " %02d: %02d: %02d\n", copia_hora.hor, copia_hora.min, copia_hora.seg);
	SeriePuts (cadena);
}

void RIT_IRQHandler(void)
{
	hora_act.useg++;
	if (hora_act.useg == 1000){
		hora_act.useg = 0;
		hora_act.seg++;
		Board_LED_Toggle(5);
		if(hora_act.seg == 60){
			hora_act.seg = 0;
			hora_act.min ++;
			if(hora_act.min == 60){
				hora_act.min = 0;
				hora_act.hor ++;
				if(hora_act.hor == 24){
					hora_act.hor = 0;
				}
			}
		}
	}
	Chip_RIT_ClearInt(LPC_RITIMER);
}

/*==================[external functions definition]==========================*/

int main(void)
{
	initHardware(); /* Inicializa el Hardware del microcontrolador */
	InitTimer(); /* Inicializa timer RIT */
	InitSerie(); /* Inicializa puerto serie */

	NVIC_EnableIRQ(RITIMER_IRQn);

	for(;;){
		ImprimeHora();
	}
}

/*==================[end of file]============================================*/
