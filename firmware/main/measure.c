#include "measure.h"
#define TAG "MEASUREMENT"

void measure_interrupt(void * args) {
    adc_continuous_handle_t adc_handle = *((adc_continuous_handle_t *)args);
    adc_continuous_stop(adc_handle);
    adc_continuous_deinit(adc_handle);
}

void measure(float duration) {
    /*
    sensor_index = 0;
    for (int i = 0; i < SENSOR_COUNT; i++) {
        for (int j = 0; j < SAMPLE_COUNT; j++) {
            sensor_reading[i][j] = 0;
        }
    }
    esp_timer_create_args_t timer_args = {
        .callback = &measure_interrupt,
        .dispatch_method = ESP_TIMER_ISR,
        .name = "MEASURMENT",
        .skip_unhandled_events = false,
    };
    esp_timer_handle_t timer_handle;
    esp_timer_create(&timer_args, &timer_handle);

    ESP_LOGI(TAG, "Starting Timer");
    esp_timer_start_periodic(timer_handle, (int)(1000000/SAMPLE_RATE));

    vTaskDelay(pdMS_TO_TICKS((int)(1000*duration)));
    ESP_LOGI(TAG, "Stopping Timer, got to idx: %d", sensor_index);
    esp_timer_stop(timer_handle);
    */

    adc_continuous_handle_t adc_handle = setup_adc();
    // esp_timer_handle_t timer_handle;
    // esp_timer_create_args_t timer_args = {
    //     .callback = &measure_interrupt,
    //     .dispatch_method = ESP_TIMER_TASK,
    //     .name = "MEASURMENT",
    //     .skip_unhandled_events = false,
    //     .arg = &adc_handle,
    // };
    // esp_timer_create(&timer_args, &timer_handle);
    
    int64_t start_time;
    int64_t elapsed;
    start_time = esp_timer_get_time();
    start_data_collection(adc_handle);
    // esp_timer_start_once(timer_handle, (int)(1000000*duration));
    vTaskDelay(pdMS_TO_TICKS((int)(1000*duration)));
    adc_continuous_stop(adc_handle);
    elapsed = esp_timer_get_time() - start_time;
    printf("Elapsed Time: %d\n", (int32_t)elapsed);
    // printf("sample_freq: %u\n", adc_handle->hal_digi_ctrlr_cfg.sample_freq_hz);
    ESP_LOGI(TAG, "Stopping Timer, got to idx: %d", sensor_index);
    // ESP_LOGI(TAG, "Elapsed Time: %d", stop-start);
    adc_continuous_deinit(adc_handle);

}

adc_continuous_handle_t setup_adc() {
    if (sensor_reading == NULL) {
        sensor_reading = (uint16_t *)heap_caps_malloc(sizeof(sensor_reading), MALLOC_CAP_SPIRAM);
    }
    adc_digi_pattern_config_t configs[SENSOR_COUNT];
    for (int i = 0; i < SENSOR_COUNT; i++) {
        adc_digi_pattern_config_t config = {
            .atten = ADC_ATTEN_DB_0,
            .channel = sensor_channels[i],
            .unit = ADC_UNIT_1,
            .bit_width = ADC_BITWIDTH_12,
        };
        configs[i] = config;
    }
    adc_continuous_config_t adc_config = {
        .pattern_num = SENSOR_COUNT,
        .adc_pattern = configs,
        .sample_freq_hz = SAMPLE_RATE*OVERSAMPLE_RATIO*SENSOR_COUNT,
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE1,   
    };

    adc_continuous_handle_cfg_t adc_handle_config = {
    .max_store_buf_size = SOC_ADC_DIGI_RESULT_BYTES*SENSOR_COUNT*OVERSAMPLE_RATIO*100,
    .conv_frame_size = SOC_ADC_DIGI_RESULT_BYTES*SENSOR_COUNT*OVERSAMPLE_RATIO*12,
    };

    adc_continuous_handle_t adc_handle;
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_handle_config, &adc_handle));

    adc_continuous_config(adc_handle, &adc_config);

    return adc_handle;
}

bool adc_callback(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data) {
    uint16_t conversion_count = edata->size/SOC_ADC_DIGI_RESULT_BYTES;
    assert(edata->size%SOC_ADC_DIGI_RESULT_BYTES == 0);
    assert(conversion_count ==SENSOR_COUNT*OVERSAMPLE_RATIO*12);
    // esp_rom_printf("conversions %d\n", conversion_count);
    adc_digi_output_data_t* conversions = (adc_digi_output_data_t*)edata->conv_frame_buffer;

    for (int j = 0; j < conversion_count/(OVERSAMPLE_RATIO*SENSOR_COUNT); j++) {

        uint32_t sensor_sums[SENSOR_COUNT] = {0};
        uint8_t sensor_reading_count[SENSOR_COUNT] = {0};

        for (int i = 0; i < OVERSAMPLE_RATIO*SENSOR_COUNT; i++) {
            sensor_sums[conversions[i].type1.channel] = conversions[i].val;
            sensor_reading_count[conversions[i].type1.channel]++;
        }

        for (int i = 0; i < SENSOR_COUNT; i++) {
            sensor_reading[SAMPLE_COUNT*i + sensor_index] = (uint16_t)(sensor_sums[i]/OVERSAMPLE_RATIO);
        }
        sensor_index = (sensor_index +1) % SAMPLE_COUNT;
        conversions += OVERSAMPLE_RATIO*SENSOR_COUNT;
    }

    return true;
}

void start_data_collection(adc_continuous_handle_t adc_handle) {
    adc_continuous_evt_cbs_t adc_cb_config = {
        .on_conv_done = &adc_callback,
    };
    sensor_index = 0;
    adc_continuous_register_event_callbacks(adc_handle, &adc_cb_config, NULL);

    adc_continuous_start(adc_handle);
}