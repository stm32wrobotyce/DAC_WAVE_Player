/*
 * wave_player.h
 *
 *  Created on: 7 lip 2021
 *      Author: piotr
 */

#ifndef INC_WAVE_PLAYER_H_
#define INC_WAVE_PLAYER_H_

#define FIRST_HALF_OF_BUFFER		1
#define SECOND_HALF_OF_BUFFER		2

#define AUDIO_BUFFER_SIZE			512
#define BYTES_IN_AUDIO_BUFFER_SIZE	AUDIO_BUFFER_SIZE*2

struct wave_player_s
{
	TIM_HandleTypeDef *htim;
	DAC_HandleTypeDef *hdac;

    uint8_t *data_pointer;
	uint16_t buffer[AUDIO_BUFFER_SIZE];
	volatile uint32_t byte_counter;
	union wave_file_header file_hdr;
};

void wave_player_init(TIM_HandleTypeDef *_htim, DAC_HandleTypeDef *_hdac);
void wave_player_start(uint8_t *file);
int8_t wave_player_read_header(uint8_t *file);
void wave_player_set_timer_arr(uint32_t sample_rate);
void wave_player_prepare_first_buffer(void);
void wave_player_prepare_half_buffer(uint8_t half_number);

#endif /* INC_WAVE_PLAYER_H_ */
