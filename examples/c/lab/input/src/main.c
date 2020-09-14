#include "board.h"

int main(void){

    SystemCoreClockUpdate();
    Board_Init();

    while(1) {
      Board_LED_Set(LED_RED, Board_GPIO_GetStatus(BOARD_GPIO_0));
      Board_LED_Set(LED_1, Board_GPIO_GetStatus(BOARD_GPIO_1)); /* Yellow */
      Board_LED_Set(LED_2, Board_GPIO_GetStatus(BOARD_GPIO_2)); /* Red */
      Board_LED_Set(LED_3, Board_GPIO_GetStatus(BOARD_GPIO_3)); /* Green */
   }
   return 0 ;
}
