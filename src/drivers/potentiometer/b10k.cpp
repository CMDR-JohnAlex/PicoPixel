#include "b10k.hpp"

#include <hardware/gpio.h>
#include <hardware/adc.h>

namespace PicoPixel {
	namespace B10kDriver {
		void InitializeB10k(B10kData* potentiometer, uint8_t gpio, uint8_t adc) {
			potentiometer->gpio	= gpio;
			potentiometer->adc	= adc;

			// TODO: Check if initializing ADC multiple times causes issues.
			adc_init();
			adc_gpio_init(potentiometer->gpio);
		}

		void DeinitializeB10k(B10kData* potentiometer) {
			// Not really necessary
			adc_select_input(0);
		}

		int ReadB10k(B10kData* potentiometer) {
			adc_select_input(potentiometer->adc);
			return adc_read();
		}
	}
}