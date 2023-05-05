#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/ringbuf.h"
#include <string.h>

#include "esp_log.h"
#include "esp_spp_api.h"
#include "messages/messages.h"
#include "messages/message_heartbeat.h"
#include "messages/message_measure.h"
#include "messages/message_measure_result.h"
#include "measure.h"

typedef struct {
    RingbufHandle_t rx_buffer;
    uint32_t handle;
} shutter_timer_task_cfg_t;


void shutter_timer_task(void *arg);

message_t handle_heartbeat(message_heartbeat_t msg);

message_t handle_measure(message_measure_t msg);