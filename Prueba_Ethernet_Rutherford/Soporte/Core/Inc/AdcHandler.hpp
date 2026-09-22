/*
 * AdcHandler.hpp
 *
 *  Created on: Dec 9, 2024
 *      Author: Alan
 */

#ifndef INC_ADCHANDLER_HPP_
#define INC_ADCHANDLER_HPP_

//#include <iostream>
//#include <cstdint>
#include "main.h"
#include <math.h>

class AdcHandler {
public:
	AdcHandler(ADC_HandleTypeDef *_hadc1);
	virtual ~AdcHandler();

	uint32_t read(uint32_t channel);
	float maping(float x, float in_min, float in_max,
			float out_min, float out_max);

private:
	ADC_HandleTypeDef *hadc1;
};

#endif /* INC_ADCHANDLER_HPP_ */
