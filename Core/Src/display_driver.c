/*
 * display_driver.c
 *
 *  Created on: 28-Sept-2026
 *      Author: Parth
 */

// display_driver.c
#include "display_driver.h"
#include "main.h"          // pin names (LCD_CS, LCD_DC, LCD_RST) from CubeMX

#define DMAMUX_REQ_SPI1_TX   17     // request ID from the ST headers (LL_DMAMUX_REQ_SPI1_TX = 0x11)

extern SPI_HandleTypeDef hspi1;

static volatile uint8_t dma_busy = 0;
static volatile uint32_t dma_error_count = 0;

void display_wait_ready(void){
	while (dma_busy){}
}

uint32_t display_dma_error_count(void){
	return dma_error_count;
}

void display_command(uint8_t cmd) {
	display_wait_ready();	//Wait for the DMA transfer to complete if already ongoing

    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);  // DC low = command
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);  // CS low = select
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);    // CS high = deselect
}

void display_data(uint8_t data) {
	display_wait_ready();	//Wait for the DMA transfer to complete if already ongoing

    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);    // DC high = data
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    display_command(0x2A);                       // CASET (columns)
    display_data(x0 >> 8); display_data(x0 & 0xFF);
    display_data(x1 >> 8); display_data(x1 & 0xFF);

    display_command(0x2B);                       // PASET (rows)
    display_data(y0 >> 8); display_data(y0 & 0xFF);
    display_data(y1 >> 8); display_data(y1 & 0xFF);

    display_command(0x2C);                       // RAMWR
}

void draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    set_window(x, y, x, y);
    display_data(color >> 8);
    display_data(color & 0xFF);
}

void draw_hline(uint16_t x, uint16_t y, uint16_t length, uint16_t color){
	set_window(x, y, x + length - 1, y);

	uint8_t hline[length * 2];

	for(uint16_t i=0; i < length; i++){
		hline[i*2] = color >> 8;
		hline[i*2 + 1] = color & 0xFF;
	}

	display_data_buffer(hline, sizeof(hline));
}

void draw_vline(uint16_t x, uint16_t y, uint16_t length, uint16_t color){
	set_window(x, y, x, y + length - 1);

	uint8_t vline[length * 2];

	for(uint16_t i=0; i < length; i++){
		vline[i*2] = color >> 8;
		vline[i*2 + 1] = color & 0xFF;
	}

	display_data_buffer(vline, sizeof(vline));
}

void fill_screen(uint16_t color) {
    uint8_t line[LCD_WIDTH * 2];               // one row of RGB565 = 480 bytes

    for (uint16_t i = 0; i < LCD_WIDTH; i++){
    	line[i*2] = color >> 8;					//High byte
    	line[i*2 + 1] = color & 0xFF;			//Low byte
    }

    set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    for (uint16_t row = 0; row < LCD_HEIGHT; row++) {
       display_data_buffer(line, sizeof(line));
    }
}

void display_data_buffer(const uint8_t *buf, uint16_t len) {
	display_wait_ready();	//Wait for the DMA transfer to complete if already ongoing

    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);    // DC high = data
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, (uint8_t *)buf, len, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

void display_init(void) {
    // Hardware reset
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(120);

    display_command(0x01);       // SWRESET
    HAL_Delay(150);

    display_command(0x11);       // SLPOUT
    HAL_Delay(120);

    display_command(0x3A);       // COLMOD
    display_data(0x55);          // RGB565

    display_command(0x36);       // MADCTL
    display_data(0x48);          // orientation + BGR for this panel

    // No INVON (0x21): this panel must stay non-inverted

    display_command(0x13);       // NORON
    display_command(0x29);       // DISPON
    HAL_Delay(20);
}

void display_dma_init(void){
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	(void)RCC->AHBENR;			// read it back: gives the clock a moment to start

	DMAMUX1_Channel0->CCR = (DMAMUX1_Channel0->CCR & ~DMAMUX_CxCR_DMAREQ_ID) | DMAMUX_REQ_SPI1_TX;

	HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

void display_data_buffer_dma(const uint8_t *buf, uint16_t len){
	display_wait_ready();			// never start while the previous one is running

	dma_busy = 1; 					// set DMA busy flag

	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);		// DC high = data
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);	// CS low = select

	//The channel must be off while it is being configured
	DMA1_Channel1->CCR &= ~DMA_CCR_EN;
	DMA1->IFCR = DMA_IFCR_CGIF1;				//clear any old flags for channel 1

	//Where from, where to, how many
	DMA1_Channel1->CPAR = (uint32_t)&SPI1->DR;	// peripheral address: the SPI data register
	DMA1_Channel1->CMAR = (uint32_t)buf;		// memory address: start of the buffer
	DMA1_Channel1->CNDTR = len;					// number of items (bytes) to move

	// Direction: memory -> peripheral. Memory address increments, peripheral does not. Interrupt on complete. Interrupt on Error.
	// Everything else stays 0: 8-bit sizes, no circular mode, low priority.
	DMA1_Channel1->CCR = DMA_CCR_DIR | DMA_CCR_MINC | DMA_CCR_TCIE | DMA_CCR_TEIE;

	DMA1_Channel1->CCR |= DMA_CCR_EN;				// start listening for SPI requests

    if (!(SPI1->CR1 & SPI_CR1_SPE)) {
        SPI1->CR1 |= SPI_CR1_SPE;                 	// make sure the SPI itself is on
    }

    SPI1->CR2 |= SPI_CR2_TXDMAEN;                 	// SPI now asks DMA for each byte -> transfer begins
}

void DMA1_Channel1_IRQHandler(void){

	uint32_t isr = DMA1->ISR;

	if(isr & (DMA_ISR_TCIF1 | DMA_ISR_TEIF1)){
		DMA1->IFCR = DMA_IFCR_CGIF1;				//clear the flag

		if(isr & DMA_ISR_TEIF1){
			dma_error_count++;
		}
		while (SPI1->SR & SPI_SR_FTLVL) { }         // SPI transmit FIFO empty
		while (SPI1->SR & SPI_SR_BSY)   { }         // SPI has finished shifting the last byte out

		HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);    // only now deselect

		// Clean up for the next user of SPI1
		SPI1->CR2 &= ~SPI_CR2_TXDMAEN;
		DMA1_Channel1->CCR &= ~DMA_CCR_EN;
	    while (SPI1->SR & SPI_SR_FRLVL) {(void)*(volatile uint8_t *)&SPI1->DR;}	// drain received bytes
	    (void)SPI1->SR;								// finishes clearing OVR

	    dma_busy = 0;
	}






}

