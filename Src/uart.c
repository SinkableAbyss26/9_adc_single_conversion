/*
 * uart.c
 *
 *  Created on: Feb 26, 2024
 *      Author: fmenc
 */

#include "uart.h"

#define GPIOAEN		(1U<<0)
#define UART2EN		(1U<<17)

#define CR1_RE		(1U<<2)
#define CR1_TE		(1U<<3)
#define CR1_UE		(1U<<13)

#define SR_RXNE		(1U<<5)
#define SR_TXE		(1U<<7)

#define SYS_FREQ	16000000
#define APB1_CLK	SYS_FREQ

#define UART_BAUDRATE	115200

static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PeriphClk, uint32_t Baudrate);
static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t Baudrate);

void uart2_tx_init(void);
void uart2_write(int ch);

/* Custom putchar function for output operations	*/
int __io_putchar(int ch)
{
	/*	Delegate the actual output to the uart2_write function	*/
	uart2_write(ch);

	/*	Return the original character	*/
	return ch;
}

void uart2_rxtx_init(void)
{
	/***************	Configure UART GPIO pin	*********************/
	/*	Enable clock access to GPIOA	*/
	RCC->AHB1ENR |= GPIOAEN;

	/*	Set PA2 to alternate function mode	*/
	GPIOA->MODER &= ~(1U<<4);
	GPIOA->MODER |= (1U<<5);

	/*	Set PA2 alternate function type to UART_TX (AF07)	*/
	GPIOA->AFR[0] |= (1U<<8);
	GPIOA->AFR[0] |= (1U<<9);
	GPIOA->AFR[0] |= (1U<<10);
	GPIOA->AFR[0] &= ~(1U<<11);

	/*	Set PA3 to alternate function mode	*/
	GPIOA->MODER &= ~(1U<<6);
	GPIOA->MODER |= (1U<<7);

	/*	Set PA3 alternate function type to UART_RX (AF07)	*/
	GPIOA->AFR[0] |= (1U<<12);
	GPIOA->AFR[0] |= (1U<<13);
	GPIOA->AFR[0] |= (1U<<14);
	GPIOA->AFR[0] &= ~(1U<<15);


	/***************	Configure UART module	*********************/
	/*	Enable clock access to UART2	*/
	RCC->APB1ENR |= UART2EN;

	/*	Configure the UART baud rate	*/
	uart_set_baudrate(USART2, APB1_CLK, UART_BAUDRATE);

	/*	Configure the transfer direction	*/
	USART2->CR1 = (CR1_TE | CR1_RE);

	/*	Enable UART module	*/
	USART2->CR1 |= CR1_UE;
}

void uart2_tx_init(void)
{
	/***************	Configure UART GPIO pin	*********************/
	/*	Enable clock access to GPIOA	*/
	RCC->AHB1ENR |= GPIOAEN;

	/*	Set PA2 to alternate function mode	*/
	GPIOA->MODER &= ~(1U<<4);
	GPIOA->MODER |= (1U<<5);

	/*	Set PA2 alternate function type to UART_TX (AF07)	*/
	GPIOA->AFR[0] |= (1U<<8);
	GPIOA->AFR[0] |= (1U<<9);
	GPIOA->AFR[0] |= (1U<<10);
	GPIOA->AFR[0] &= ~(1U<<11);


	/***************	Configure UART module	*********************/
	/*	Enable clock access to UART2	*/
	RCC->APB1ENR |= UART2EN;

	/*	Configure the UART baud rate	*/
	uart_set_baudrate(USART2, APB1_CLK, UART_BAUDRATE);

	/*	Configure the transfer direction	*/
	USART2->CR1 = CR1_TE;

	/*	Enable UART module	*/
	USART2->CR1 |= CR1_UE;
}

char uart2_read(void)
{
	/*	Make sure the receive data register is not 	empty	*/
	while (!(USART2->SR & SR_RXNE)){}

	/*	Read data	*/
	return USART2->DR;
}

void uart2_write(int ch)
{
	/*	Make sure the transmit data register is empty	*/
	while (!(USART2->SR & SR_TXE)){}

	/*	Write to transmit data register	*/
	USART2->DR = (ch & 0xFF);

}

static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PeriphClk, uint32_t Baudrate)
{
	/*	 Set the BRR (Baud Rate Register) using the result of compute_uart_bd */
	USARTx->BRR = compute_uart_bd(PeriphClk, Baudrate);
}

static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t Baudrate)
{
	/*	Compute and return the value to be set in the BRR for a given baud rate	*/
	return ((PeriphClk + (Baudrate/2U))/Baudrate);
}
