#ifndef MAIN_H
#define	MAIN_H

FATFS file_system;
uint8_t reset;
uint8_t sd_initialized;
uint8_t file_num;

uint8_t clk_init(void);
void shutdown(void);
uint8_t delay_clks(uint8_t delay);
uint8_t delay_ms(uint16_t delay);
void beep(uint8_t beeps);
uint8_t init_sd_card(void);
uint8_t disable_sd_card(void);
uint8_t open_file(uint8_t file_num);
uint8_t read_file(void);
uint8_t play(void);
uint8_t loop(void);
int main(void);

#endif	/* MAIN_H */

