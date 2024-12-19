#include "uart.h"
#include "rcc.h"
#include "nvic.h"
#include "gpio.h"

static volatile command_t last_command = CMD_NONE;
void UART_clock_enable(USART_TypeDef * UARTx) { 
    if (UARTx == USART1) {
        *RCC_APB2ENR |= RCC_APB2ENR_USART1EN;  // Habilita el reloj para USART1
    } else if (UARTx == USART2) {
        *RCC_APB1ENR1 |= RCC_APB1ENR1_USART2EN;  // Habilita el reloj para USART2
    } else if (UARTx == USART3) {
        *RCC_APB1ENR1 |= RCC_APB1ENR1_USART3EN;  // Habilita el reloj para USART3
    }
}

// void usart2_init(void)
// {
//     configure_gpio_for_usart();

//     *RCC_APB1ENR1 |= RCC_APB1ENR1_USART2EN;

    // TODO: Configurar UART2

//     // Activar interrupción de RXNE
//     USART2->CR1 |= USART_CR1_RXNEIE; 
//     NVIC->ISER[1] |= (1 << 6);
// }
void usart2_init(void) {
    // Habilitar el reloj para USART2
    UART_clock_enable(USART2);

    // Desactivar USART2 para configuración
    USART2->CR1 &= ~USART_CR1_UE;

    // Configurar longitud de datos a 8 bits (limpiar bit M)
    USART2->CR1 &= ~USART_CR1_M;

    // Configurar 1 bit de parada (limpiar bits STOP en CR2)
    USART2->CR2 &= ~USART_CR2_STOP;

    // Configurar sin paridad (limpiar bit PCE)
    USART2->CR1 &= ~USART_CR1_PCE;

    // Configuración de sobre muestreo por 16 (limpiar bit OVER8)
    USART2->CR1 &= ~USART_CR1_OVER8;

    // Establecer el baud rate a 9600 usando la frecuencia del APB1 (suponiendo 4 MHz)
    USART2->BRR = BAUD_9600_4MHZ;

    // Habilitar transmisión y recepción
    USART2->CR1 |= (USART_CR1_TE | USART_CR1_RE);

    // Habilitar USART2
    USART2->CR1 |= USART_CR1_UE;

    // Verificar que USART2 esté listo para transmitir
    while ((USART2->ISR & USART_ISR_TEACK) == 0);

    // Verificar que USART2 esté listo para recibir
    while ((USART2->ISR & USART_ISR_REACK) == 0);

    // Activar la interrupción de RXNE
    USART2->CR1 |= USART_CR1_RXNEIE;

    // Habilitar la interrupción en el NVIC para USART2
    NVIC->ISER[1] |= (1 << 6);  // Interrupción en el vector IRQ 38 (USART2)
}
void usart2_send_string(const char *str)
{
    while (*str) {
        while (!(USART2->ISR & USART_ISR_TXE));
        USART2->TDR = *str++;
    }
}

command_t usart2_get_command(void)
{
    command_t cmd = last_command;
    last_command = CMD_NONE;
    return cmd;
}


void USART2_IRQHandler(void)
{
    uint32_t isr = USART2->ISR;
    if (isr & USART_ISR_RXNE) {
        char command = USART2->RDR;
        if (command == 'O') {
            last_command = CMD_OPEN;
        } else if (command == 'C') {
            last_command = CMD_CLOSE;
        }
    }
}

