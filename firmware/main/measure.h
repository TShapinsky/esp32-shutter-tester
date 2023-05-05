#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "esp_log.h"
// #include "driver/adc.h"
#include "esp_adc/adc_continuous.h"
#include "soc/soc_caps.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define SAMPLE_COUNT 200000
#define SAMPLE_RATE 65000
#define OVERSAMPLE_RATIO 4
#define SENSOR_COUNT 4

#ifndef MEASURE_H
#define MEASURE_H

static const uint8_t sensor_channels[] = {
    ADC_CHANNEL_0,
    ADC_CHANNEL_3,
    ADC_CHANNEL_4,
    ADC_CHANNEL_5
};


static uint16_t * sensor_reading = NULL;
static uint16_t sensor_index = 0;

void measure_interrupt(void * args);
void measure(float duration);
adc_continuous_handle_t setup_adc();
bool adc_callback(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data);
void start_data_collection(adc_continuous_handle_t adc_handle);
#endif