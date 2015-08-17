/**
  ******************************************************************************
  * @file    EXTI/LED_Toggle/main.c 
  * @author  IOP Team
  * @version V1.0.0
  * @date    01-May-2015
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, WIZnet SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2015 WIZnet Co.,Ltd.</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "W7500x_gpio.h"
#include "W7500x_exti.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void GPIO_Setting(void);
void NVIC_Configuration(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  */
int main()
{
    /* set system */
    SystemInit();
//    *(volatile uint32_t *)(0x41001014) = 0x0060100; //clock setting 48MHz
    
    /* CLK OUT Set */
//    PAD_AFConfig(PAD_PA,GPIO_Pin_2, PAD_AF2); // PAD Config - CLKOUT used 3nd Function
    /* CPIO configuration */
    GPIO_Setting();

    /* NVIC Configuration */
    NVIC_Configuration();
	
    while(1)
    {
		; // waiting for External Interrupt!!
    }
}

/**
  * @brief GPIO Configuration function 
  */
void GPIO_Setting(void)
{
    GPIO_InitTypeDef GPIO_InitDef;
    EXTI_InitTypeDef EXTI_InitDef;

    /* Set to GPIO_Pin_0 to Input Port */
    GPIO_InitDef.GPIO_Pin = GPIO_Pin_0; // Set for Connecting EXTI
    GPIO_InitDef.GPIO_Mode = GPIO_Mode_IN; // Set to GPIO Mode to Input port
    GPIO_Init(GPIOA, &GPIO_InitDef); // Set to GPIOA

    /* GPIO LED(R) Setting */
    GPIO_InitDef.GPIO_Pin = GPIO_Pin_8; // Connecting GPIO_Pin_8(LED(R))
    GPIO_InitDef.GPIO_Mode = GPIO_Mode_OUT; // Set to GPIO Mode to Output Port
    GPIO_Init(GPIOC, &GPIO_InitDef); // Set to GPIOC
    PAD_AFConfig(PAD_PC,GPIO_Pin_8 , PAD_AF1); // PAD Config - LED used 2nd Function
    
    /* Set to GPIO_Pin_0 to External Interrupt Port */
    EXTI_InitDef.EXTI_Line = GPIO_Pin_0; // Connecting GPIO_Pin_0(EXTI Input)
    EXTI_InitDef.EXTI_Trigger = EXTI_Trigger_Rising; // Set to Trigger to Rising
    EXTI_Init(PAD_PA, &EXTI_InitDef); // Set to PAD_PA
    EXTI_Polarity_Set(PAD_PA,GPIO_Pin_0,EXTI_Trigger_Rising); // Set to Polarity
}

/**
  * @brief  Configure the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
void NVIC_Configuration(void)
{
    NVIC_ClearPendingIRQ(EXTI_IRQn); // Pending bit Clear
    NVIC_EnableIRQ(EXTI_IRQn);       // EXTI Interrupt Enable
}

