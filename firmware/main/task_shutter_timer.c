#include "task_shutter_timer.h"

#define TAG "Shutter Timer"

void shutter_timer_task(void *arg) {
    shutter_timer_task_cfg_t *config = (shutter_timer_task_cfg_t*) arg;

    while(true) {
        size_t received_size = 0;
        // Receive message header
        uint8_t* message_header = xRingbufferReceiveUpTo(config->rx_buffer, &received_size, pdMS_TO_TICKS(10), MSG_HEADER_SIZE);
        if (!received_size) {
            continue;
        }
        // ESP_LOGI(TAG, "RECEIVED HEADER");
        uint8_t msg_type = message_header[0];
        uint32_t msg_len = 0;
        memcpy(&msg_len, message_header + sizeof(uint8_t), sizeof(uint32_t));
        message_t msg = {
            .type = msg_type,
            .len  = msg_len,
            .body = malloc(msg_len),
        };
        // for (int i = 0; i < received_size; i++) {
        //     ESP_LOGI(TAG, "%x", message_header[i]);
        // }
        vRingbufferReturnItem(config->rx_buffer, message_header);

        // Receive message body
        uint8_t * received_bytes = xRingbufferReceiveUpTo(config->rx_buffer, &received_size, pdMS_TO_TICKS(10), (size_t)msg_len);
        if (!received_size) {
            continue;
        }
        // ESP_LOGI(TAG, "Received %d bytes", received_size);
        // ESP_LOGI(TAG, "Message Len %d bytes", msg_len);
        // for (int i = 0; i < received_size; i++) {
        //     ESP_LOGI(TAG, "%x", received_bytes[i]);
        // }
        memcpy(msg.body, received_bytes, received_size);
        vRingbufferReturnItem(config->rx_buffer, received_bytes);
        message_t response;
        switch (msg_type) {
            case MSG_ID_HEARTBEAT:
                message_heartbeat_t msg_heartbeat = msg_heartbeat_unpack(msg);
                ESP_LOGI(TAG, "Received HEARTBEAT: .echo = %d", msg_heartbeat.echo);
                response = handle_heartbeat(msg_heartbeat);
                break;
            case MSG_ID_MEASURE:
                message_measure_t msg_measure = msg_measure_unpack(msg);
                ESP_LOGI(TAG, "Received MEASURE: .duration = %f", msg_measure.duration);
                response = handle_measure(msg_measure);
                break;
        }

        send_msg(config->handle, response);
        received_size = 0;
        received_bytes = xRingbufferReceive(config->rx_buffer, &received_size, pdMS_TO_TICKS(10));
        if (received_size) {
            vRingbufferReturnItem(config->rx_buffer, received_bytes);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


message_t handle_heartbeat(message_heartbeat_t msg) {
    ESP_LOGI(TAG, "RECEIVED HEARTbEAT");
    return msg_heartbeat_pack(msg);
}

message_t handle_measure(message_measure_t msg) {

    measure(msg.duration);
    message_measure_result_t result_msg = {
        .sensor_count = SENSOR_COUNT,
        .raw_min  = malloc(sizeof(uint16_t)*SENSOR_COUNT),
        .raw_max  = malloc(sizeof(uint16_t)*SENSOR_COUNT),
        .min      = malloc(sizeof(uint16_t)*SENSOR_COUNT),
        .max      = malloc(sizeof(uint16_t)*SENSOR_COUNT),
        .fwhm     = malloc(sizeof(float)*SENSOR_COUNT),
        .exposure = malloc(sizeof(float)*SENSOR_COUNT),
        .on_25    = malloc(sizeof(float)*SENSOR_COUNT),
        .on_50    = malloc(sizeof(float)*SENSOR_COUNT),
        .on_75    = malloc(sizeof(float)*SENSOR_COUNT),
        .off_25   = malloc(sizeof(float)*SENSOR_COUNT),
        .off_50   = malloc(sizeof(float)*SENSOR_COUNT),
        .off_75   = malloc(sizeof(float)*SENSOR_COUNT),
    };
    result_msg.min[3] = 260;
    result_msg.off_50[2] = 0.5;
    result_msg = msg_measure_result_unpack(msg_measure_result_pack(result_msg));
    ESP_LOGI(TAG, "min[3] = %d", result_msg.min[3]);
    ESP_LOGI(TAG, "off_50[2] = %f", result_msg.off_50[2]);
    return msg_measure_result_pack(result_msg);
}