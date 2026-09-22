/*
 * AdcHandler.cpp
 *
 *  Created on: Dec 9, 2024
 *      Author: Alan
 */

#include <AdcHandler.hpp>

AdcHandler::AdcHandler(ADC_HandleTypeDef *_hadc1) {
	hadc1 = _hadc1;
}

AdcHandler::~AdcHandler() {
}

uint32_t AdcHandler::read(uint32_t channel) {
	ADC_ChannelConfTypeDef sConfig = { 0 };
	sConfig.Channel = channel;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;

	if (HAL_ADC_ConfigChannel(hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}

	HAL_ADC_Start(hadc1);
	HAL_ADC_PollForConversion(hadc1, HAL_MAX_DELAY);
	uint32_t value = HAL_ADC_GetValue(hadc1);
	HAL_ADC_Stop(hadc1);

	return value;

}

float AdcHandler::maping(float x, float in_min, float in_max, float out_min,
		float out_max) {
//	float result = 0;


	float result = floorf(((x - in_min) * (out_max - out_min + 1)/ (in_max - in_min + 1) + out_min)*100)/100;

	return result;
}
