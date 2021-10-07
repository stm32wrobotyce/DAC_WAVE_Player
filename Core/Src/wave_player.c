/*
 * wave_file.c
 *
 *  Created on: Jul 6, 2021
 *      Author: piotr
 */

#include "main.h"

struct wave_player_s wave_player;

void wave_player_init(TIM_HandleTypeDef *_htim, DAC_HandleTypeDef *_hdac)
{
	wave_player.htim = _htim;
	wave_player.hdac = _hdac;
}

void wave_player_start(uint8_t *file)
{
	int8_t status;

	status = wave_player_read_header(file);

	if(ERROR == status)
	{
		return;
	}

	wave_player_set_timer_arr(wave_player.file_hdr.wave_file_hdr.sample_rate);

	wave_player_prepare_first_buffer();

	HAL_TIM_Base_Start(wave_player.htim);
	HAL_DAC_Start_DMA(wave_player.hdac, DAC_CHANNEL_1, (uint32_t *)wave_player.buffer, AUDIO_BUFFER_SIZE, DAC_ALIGN_12B_L);
}

int8_t wave_player_read_header(uint8_t *file)
{
	uint32_t i;

	wave_player.data_pointer = file;

	for(i = 0; i < WAVE_FILE_HEADER_SIZE; i++)
	{
		wave_player.file_hdr.bytes[i] = *(wave_player.data_pointer+i);
	}

	if(RIFF_HDR_CONST != wave_player.file_hdr.wave_file_hdr.riff_hdr)
	{
		return ERROR;
	}

	if(WAVE_ID_CONST != wave_player.file_hdr.wave_file_hdr.wave_id)
	{
		return ERROR;
	}

	if(FMT_CONST != wave_player.file_hdr.wave_file_hdr.chunk_marker)
	{
		return ERROR;
	}

	if(CHANNEL_MONO != wave_player.file_hdr.wave_file_hdr.channels)
	{
		return ERROR;
	}

	wave_player.byte_counter = WAVE_FILE_HEADER_SIZE;

	return SUCCESS;
}

void wave_player_set_timer_arr(uint32_t sample_rate)
{
	uint32_t arr;

	arr = HAL_RCC_GetPCLK1Freq() / sample_rate;

	__HAL_TIM_SET_AUTORELOAD(wave_player.htim, arr - 1);
}

void wave_player_prepare_first_buffer(void)
{
	int32_t i;
	int16_t audio_sample;

	for(i = 0; i < AUDIO_BUFFER_SIZE; i++)
	{
		audio_sample = (*(wave_player.data_pointer+(wave_player.byte_counter+1)) << 8) | *(wave_player.data_pointer+wave_player.byte_counter);

		wave_player.buffer[i] = (audio_sample + 32768);

		wave_player.byte_counter += 2;
	}
}

void wave_player_prepare_half_buffer(uint8_t half_number)
{
	int32_t i;
	int16_t audio_sample;

	if(FIRST_HALF_OF_BUFFER == half_number)
	{
		for(i = 0; i < AUDIO_BUFFER_SIZE/2; i++)
		{
			audio_sample = (*(wave_player.data_pointer+(wave_player.byte_counter+1)) << 8) | *(wave_player.data_pointer+wave_player.byte_counter);

			wave_player.buffer[i] = (audio_sample + 32768);

			wave_player.byte_counter += 2;

			if(wave_player.byte_counter >= wave_player.file_hdr.wave_file_hdr.data_size)
			{
				HAL_DAC_Stop_DMA(wave_player.hdac, DAC_CHANNEL_1);
				return;
			}
		}
	}
	else if(SECOND_HALF_OF_BUFFER == half_number)
	{
		HAL_DAC_Start_DMA(wave_player.hdac, DAC_CHANNEL_1, (uint32_t *)wave_player.buffer, AUDIO_BUFFER_SIZE, DAC_ALIGN_12B_L);

		for(i = AUDIO_BUFFER_SIZE/2; i < AUDIO_BUFFER_SIZE; i++)
		{
			audio_sample = (*(wave_player.data_pointer+(wave_player.byte_counter+1)) << 8) | *(wave_player.data_pointer+wave_player.byte_counter);

			wave_player.buffer[i] = (audio_sample + 32768);

			wave_player.byte_counter += 2;

			if(wave_player.byte_counter >= wave_player.file_hdr.wave_file_hdr.data_size)
			{
				HAL_DAC_Stop_DMA(wave_player.hdac, DAC_CHANNEL_1);
				return;
			}
		}
	}
}
