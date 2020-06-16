/*  Copyright 2020 
        codigo basado en el libro Sistemas Empotrados en tiempo real 
        José Daniel Muñoz Frías
1.5     Procesamiento Secuecial (Pagina 8)
1.5.1 Ejemplo: Un Termostato digital

se muestra un ejemplo sencillo de un sistema en tiempo real implantado mediante 
procesamiento secuencial. El sistema es un termostato para la calefacción. 
Como se puede observar, el sistema consta de cuatro tareas que se ejecutan continuamente:
- La primera verifica si hay alguna tecla pulsada (UP o DOWN) y en caso
afirmativo modifica la consigna de temperatura. Si no hay ninguna
tecla pulsada, simplemente termina de ejecutarse (no bloqueo).
- La segunda tarea realiza una medida de la temperatura de la habitación.
- La tercera ejecuta el control. Éste puede ser un control sencillo todo/nada, 
consistente en que si la temperatura es mayor que la consigna
se apaga la calefacción y si es inferior se enciende.
- Por último, la cuarta tarea se encarga de encender o apagar la cale-
facción en función de la salida de la tarea de control
 */

/*==================[inclusions]=============================================*/

#include "board.h"
#include "main.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/** @brief hardware initialization function
 *  @return none
 */
static void initHardware(void);

/** @brief delay function
 * @param t desired milliseconds to wait
 */
static  void delay(uint32_t t);



/*==================[internal data definition]===============================*/
/** @brief used for delay counter */
static volatile uint32_t pausems_count;

int temp = 0;
int consigna = 10;
int convertido = 0;
int fin_timer = 0;
int calefaccion = OFF;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void initHardware(void)
{
    Board_Init();
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000);
}

static void delay(uint32_t t)
{
    pausems_count = t;
    while (pausems_count != 0) {
        __WFI();
    }
}


static uint8_t LeerTeclado(void)
{
    if (Board_TEC_GetStatus(BOARD_TEC_1) == 0) return DOWN;
    if (Board_TEC_GetStatus(BOARD_TEC_2) == 0) return UP;
    else return 0;

}

static int Convirtiendo() {
	if (convertido == 0){
	    return 0;
	}
	else {
		convertido = 0;
		return 1;
	}
}

static void LanzarConversionAD(void){
	}

static int LeerConversorAD(void){ //simulado
	if (calefaccion == 1) {
		temp = temp + 1;
	}
	else {
		temp = temp - 1;
	}
	return temp;
}
//////////////////////////////////////////////////////////

static void LeerTeclas(void)
{
    uint8_t tecla;
	tecla = LeerTeclado();
	if(tecla == UP ){
		consigna ++;
	}
	if(tecla == DOWN ){
		consigna --;
	}
}

static void MedirTemperatura(void){
	LanzarConversionAD();
	while ( Convirtiendo());
	/* Esperar EOC */
	temp = LeerConversorAD();

}

static void Controlar(void){
	if(temp < consigna ){
		calefaccion = ON;
		Board_LED_Set(LED_ROJO, TRUE); //enciende led
		}
	else if(temp > consigna + HISTERESIS ){
		calefaccion = OFF;
		Board_LED_Set(LED_ROJO, FALSE); //apaga led
		}
}

static void ImprimirTemp(void)
{
	printf (" Temp:%02d C - Consigna:%02d - calefaccion:%01d\n", temp,consigna,calefaccion);
}


static void TareaInactiva (void)
{
	while ( ! fin_timer );
	fin_timer = 0;
}
/*==================[external functions definition]==========================*/

void SysTick_Handler(void)
{
     static int ms;
     static int seg;
     ms++;
     if (ms == 100){
         convertido = 1;
         ms = 0;
         seg ++;
         if (seg == 10) {
            fin_timer = 1;
            seg = 0;
         }
      }
 
    if(pausems_count > 0) pausems_count--;
}

int main(void)
{
    initHardware();

    while (1)
    {
        Board_LED_Toggle(LED_VERDE);
		LeerTeclas();
		MedirTemperatura();
		Controlar();
		ImprimirTemp();
        TareaInactiva();
        //delay(DELAY_MS);
    }
}



/*==================[end of file]============================================*/
