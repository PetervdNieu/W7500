/**
  ******************************************************************************
  * @file    /DMA/MemoryToMemory/main.c 
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
#include <stdio.h>
#include "W7500x_dma.h"
#include "W7500x_uart.h"
#include "W7500x_crg.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
int cnum = 1;
uint32_t rx_flag =0 ;
int result=0;

uint32_t sysclock;

static __IO uint32_t TimingDelay;

volatile extern int dma_done_irq_occurred;
volatile extern int dma_done_irq_expected;
volatile extern int dma_error_irq_occurred;
volatile extern int dma_error_irq_expected;

volatile unsigned int source_data_array[4];  /* Data array for memory DMA test */
volatile unsigned int dest_data_array[4];    /* Data array for memory DMA test */

UART_InitTypeDef UART_InitStructure;

/* Private function prototypes -----------------------------------------------*/
void delay(__IO uint32_t milliseconds);
void TimingDelay_Decrement(void);

int  dma_simple_test(void);
int  dma_interrupt_test(void);
int  dma_event_test(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
int main (void)
{
	  /* Set Systme init */
    SystemInit();
//    *(volatile uint32_t *)(0x41001014) = 0x0060100; //clock setting 48MHz
    
    /* CLK OUT Set */
//    PAD_AFConfig(PAD_PA,GPIO_Pin_2, PAD_AF2); // PAD Config - CLKOUT used 3nd Function

    /* Get System Clock */
    sysclock = GetSystemClock();     

    /* SysTick_Config */
    SysTick_Config((sysclock/1000));


    /* External Clock */
    CRG_PLL_InputFrequencySelect(CRG_OCLK);
  
	
    /* UART0 Init */
    UART_StructInit(&UART_InitStructure);
    UART_Init(UART1,&UART_InitStructure);
	
    /* wait for test */ 
    delay(100);

    dma_done_irq_expected = 0;
    dma_done_irq_occurred = 0;
    dma_error_irq_expected = 0;
    dma_error_irq_occurred = 0;
    dma_data_struct_init();
    dma_init();

    result += dma_simple_test();
    //result += dma_interrupt_test();
    //result += dma_event_test();
    if (result==0) {
        printf ("\n** TEST PASSED **\n");
    } else {
        printf ("\n** TEST FAILED **, Error code = (0x%x)\n", result);
    }
    return 0;
}

/* --------------------------------------------------------------- */
/*  Simple software DMA test                                       */
/* --------------------------------------------------------------- */
int dma_simple_test(void)
{
  int return_val=0;
  int err_code=0;
  int i;
  unsigned int current_state;


  printf("uDMA simple test");
  DMA->CHNL_ENABLE_SET = (1<<cnum); /* Enable channel 0 */

  /* setup data for DMA */
  for (i=0;i<4;i++) {
    source_data_array[i] = i;
    dest_data_array[i]   = 0;
  }
	
  dma_memory_copy (cnum, (unsigned int) &source_data_array[0],(unsigned int) &dest_data_array[0], 2, 4);
  do { /* Wait until PL230 DMA controller return to idle state */
    current_state = (DMA->DMA_STATUS >> 4)  & 0xF;
  } while (current_state!=0);

  for (i=0;i<4;i++) 
  {
    /* Debugging printf: */
    /*printf (" - dest[i] = %x\n", dest_data_array[i]);*/
    if (dest_data_array[i]!= source_data_array[i])
    {
      printf ("ERROR:dest_data_array[%d], expected %x, actual %x\n", i, source_data_array[i], dest_data_array[i]);
      err_code |= (1<<i);
    }
  }

  /* Generate return value */
  if (err_code != 0) {
    printf ("ERROR : simple DMA failed (0x%x)\n", err_code);
    return_val=1;
  } else {
    printf ("-Passed");
  }

  return(return_val);
}
/* --------------------------------------------------------------- */
/*  Simple DMA interrupt test                                      */
/* --------------------------------------------------------------- */
int dma_interrupt_test(void)
{
  int return_val=0;
  int err_code=0;
  int i;
  unsigned int current_state;


  printf("DMA interrupt test");
  printf("- DMA done");

  DMA->CHNL_ENABLE_SET = (1<<cnum); /* Enable channel 0 */

  /* setup data for DMA */
  for (i=0;i<4;i++) {
    source_data_array[i] = i;
    dest_data_array[i]   = 0;
  }

  dma_done_irq_expected = 1;
  dma_done_irq_occurred = 0;
  NVIC_ClearPendingIRQ(DMA_IRQn);
  NVIC_EnableIRQ(DMA_IRQn);

  dma_memory_copy (cnum, (unsigned int) &source_data_array[0],(unsigned int) &dest_data_array[0], 2, 4);
    delay(1000);
  /* Can't guarantee that there is sleep support, so use a polling loop */
  do { /* Wait until PL230 DMA controller return to idle state */
    current_state = (DMA->DMA_STATUS >> 4)  & 0xF;
  } while (current_state!=0);

  for (i=0;i<4;i++) {
    /* Debugging printf: */
    /*printf (" - dest[i] = %x\n", dest_data_array[i]);*/
    if (dest_data_array[i]!= i){
      printf ("ERROR:dest_data_array[%d], expected %x, actual %x\n", i, i, dest_data_array[i]);
      err_code |= (1<<i);
    }
  }

  if (dma_done_irq_occurred==0){
    printf ("ERROR: DMA done IRQ missing");
    err_code |= (1<<4);
  }

  printf("- DMA err");
  DMA->CHNL_ENABLE_SET = (1<<cnum); /* Enable channel 0 */

  /* setup data for DMA */
  for (i=0;i<4;i++) {
    source_data_array[i] = i;
    dest_data_array[i]   = 0;
  }

  dma_error_irq_expected = 1;
  dma_error_irq_occurred = 0;
  NVIC_ClearPendingIRQ(DMA_IRQn);
  NVIC_EnableIRQ(DMA_IRQn);

  /* Generate DMA transfer to invalid memory location */
  dma_memory_copy (cnum, (unsigned int) &source_data_array[0],0x1F000000, 2, 4);
    delay(1000);
  /* Can't guarantee that there is sleep support, so use a polling loop */
  do { /* Wait until PL230 DMA controller return to idle state */
    current_state = (DMA->DMA_STATUS >> 4)  & 0xF;
  } while (current_state!=0);

  if (dma_error_irq_occurred==0){
    printf ("ERROR: DMA err IRQ missing");
    err_code |= (1<<5);
  }


  /* Clear up */
  dma_done_irq_expected = 0;
  dma_done_irq_occurred = 0;
  dma_error_irq_expected = 0;
  dma_error_irq_occurred = 0;
  NVIC_DisableIRQ(DMA_IRQn);

  /* Generate return value */
  if (err_code != 0) {
    printf ("ERROR : DMA done interrupt failed (0x%x)\n", err_code);
    return_val=1;
  } else {
    printf ("-Passed");
  }

  return(return_val);
}

/* --------------------------------------------------------------- */
/*  DMA event test                                                 */
/* --------------------------------------------------------------- */
int dma_event_test(void)
{
  int return_val=0;
  int err_code=0;
  int i;
  unsigned int current_state;


  printf("DMA event test");
  printf("- DMA done event to RXEV");

  DMA->CHNL_ENABLE_SET = (1<<cnum); /* Enable channel 0 */

  /* setup data for DMA */
  for (i=0;i<4;i++) {
    source_data_array[i] = i;
    dest_data_array[i]   = 0;
  }

  dma_done_irq_expected = 1;
  dma_done_irq_occurred = 0;
  NVIC_ClearPendingIRQ(DMA_IRQn);
  NVIC_DisableIRQ(DMA_IRQn);

  /* Clear event register - by setting event with SEV and then clear it with WFE */
  __SEV();
  __WFE(); /* First WFE will not enter sleep because of previous event */

  dma_memory_copy (cnum, (unsigned int) &source_data_array[0],(unsigned int) &dest_data_array[0], 2, 4);
  __WFE(); /* This will cause the processor to enter sleep */

  /* Processor woken up */
  current_state = (DMA->DMA_STATUS >> 4)  & 0xF;
  if (current_state!=0) {
    printf ("ERROR: DMA status should be IDLE after wake up");
    err_code |= (1<<5);
  }

  for (i=0;i<4;i++) {
    /*printf (" - dest[i] = %x\n", dest_data_array[i]);*/
    if (dest_data_array[i]!= i){
      printf ("ERROR:dest_data_array[%d], expected %x, actual %x\n", i, i, dest_data_array[i]);
      err_code |= (1<<i);
    }
  }

  /* Generate return value */
  if (err_code != 0) {
    printf ("ERROR : DMA event failed (0x%x)\n", err_code);
    return_val=1;
  } else {
    printf ("-Passed");
  }

  return(return_val);
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void delay(__IO uint32_t milliseconds)
{
  TimingDelay = milliseconds;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}





