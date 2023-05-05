#include "measure.h"
typedef struct __message_measure_result_t {
    uint8_t sensor_count;
    uint16_t * raw_min;
    uint16_t * raw_max;
    uint16_t * min;
    uint16_t * max;
    float * fwhm;
    float * exposure;
    float * on_25;
    float * on_50;
    float * on_75;
    float * off_25;
    float * off_50;
    float * off_75;
} message_measure_result_t;

#define MSG_ID_MEASURE_RESULT_LEN sizeof(uint8_t)+sizeof(uint16_t)*4*SENSOR_COUNT + sizeof(float)*8*SENSOR_COUNT
#define MSG_ID_MEASURE_RESULT 0x31

static inline message_t msg_measure_result_pack(message_measure_result_t msg) {
    uint8_t buf[MSG_ID_MEASURE_RESULT_LEN] = { 0 };
    uint8_t * idx = buf;
    message_t packed_msg = {
        .len = MSG_ID_MEASURE_RESULT_LEN,
        .type = MSG_ID_MEASURE_RESULT,
        .body = buf
    };
    uint8_t sensor_count = msg.sensor_count;
    memcpy(idx, &(msg.sensor_count), sizeof(uint8_t)); idx += sizeof(uint8_t);
    
    memcpy(idx, msg.raw_min, sizeof(uint16_t)*sensor_count); idx += sizeof(uint16_t)*sensor_count;
    memcpy(idx, msg.raw_max, sizeof(uint16_t)*sensor_count); idx += sizeof(uint16_t)*sensor_count;
    memcpy(idx, msg.min, sizeof(uint16_t)*sensor_count); idx += sizeof(uint16_t)*sensor_count;
    memcpy(idx, msg.max, sizeof(uint16_t)*sensor_count); idx += sizeof(uint16_t)*sensor_count;
    memcpy(idx, msg.min, sizeof(uint16_t)*sensor_count); idx += sizeof(uint16_t)*sensor_count;

    memcpy(idx, msg.fwhm, sizeof(float)*sensor_count); idx += sizeof(float)*sensor_count;
    memcpy(idx, msg.exposure, sizeof(float)*sensor_count); idx += sizeof(float)*sensor_count;
    memcpy(idx, msg.on_25, sizeof(float)*sensor_count); idx += sizeof(float)*sensor_count;
    memcpy(idx, msg.on_25, sizeof(float)*sensor_count); idx += sizeof(float)*sensor_count;
    memcpy(idx, msg.on_75, sizeof(float)*sensor_count); idx += sizeof(float)*sensor_count;
    memcpy(idx, msg.off_25, sizeof(float)*sensor_count); idx += sizeof(float)*sensor_count;
    memcpy(idx, msg.off_50, sizeof(float)*sensor_count); idx += sizeof(float)*sensor_count;
    memcpy(idx, msg.off_75, sizeof(float)*sensor_count); idx += sizeof(float)*sensor_count;

    return packed_msg;
}

static inline message_measure_result_t msg_measure_result_unpack(message_t msg) {
    uint8_t sensor_count = *msg.body;
    message_measure_result_t unpacked_msg = {
        .sensor_count = sensor_count,
        .raw_min  = (uint16_t *) malloc(sizeof(uint16_t)*sensor_count),
        .raw_max  = (uint16_t *) malloc(sizeof(uint16_t)*sensor_count),
        .min      = (uint16_t *) malloc(sizeof(uint16_t)*sensor_count),
        .max      = (uint16_t *) malloc(sizeof(uint16_t)*sensor_count),
        .fwhm     = (float *) malloc(sizeof(float)*sensor_count),
        .exposure = (float *) malloc(sizeof(float)*sensor_count),
        .on_25    = (float *) malloc(sizeof(float)*sensor_count),
        .on_50    = (float *) malloc(sizeof(float)*sensor_count),
        .on_75    = (float *) malloc(sizeof(float)*sensor_count),
        .off_25   = (float *) malloc(sizeof(float)*sensor_count),
        .off_50   = (float *) malloc(sizeof(float)*sensor_count),
        .off_75   = (float *) malloc(sizeof(float)*sensor_count),
    };

    uint8_t * idx = msg.body + sizeof(uint8_t);

    memcpy( unpacked_msg.raw_min, idx, sizeof(uint16_t)*sensor_count); idx += sizeof(uint16_t)*sensor_count;
    memcpy( unpacked_msg.raw_max, idx, sizeof(uint16_t)*sensor_count); idx += sizeof(uint16_t)*sensor_count;
    memcpy( unpacked_msg.min, idx, sizeof(uint16_t)*sensor_count); idx += sizeof(uint16_t)*sensor_count;
    memcpy( unpacked_msg.max, idx, sizeof(uint16_t)*sensor_count); idx += sizeof(uint16_t)*sensor_count;
    memcpy( unpacked_msg.min, idx, sizeof(uint16_t)*sensor_count); idx += sizeof(uint16_t)*sensor_count;

    memcpy( unpacked_msg.fwhm, idx, sizeof(float)*sensor_count); idx += sizeof(float)*sensor_count;
    memcpy( unpacked_msg.exposure, idx, sizeof(float)*sensor_count); idx += sizeof(float)*sensor_count;
    memcpy( unpacked_msg.on_25, idx, sizeof(float)*sensor_count); idx += sizeof(float)*sensor_count;
    memcpy( unpacked_msg.on_25, idx, sizeof(float)*sensor_count); idx += sizeof(float)*sensor_count;
    memcpy( unpacked_msg.on_75, idx, sizeof(float)*sensor_count); idx += sizeof(float)*sensor_count;
    memcpy( unpacked_msg.off_25, idx, sizeof(float)*sensor_count); idx += sizeof(float)*sensor_count;
    memcpy( unpacked_msg.off_50, idx, sizeof(float)*sensor_count); idx += sizeof(float)*sensor_count;
    memcpy( unpacked_msg.off_75, idx, sizeof(float)*sensor_count); idx += sizeof(float)*sensor_count;

    return unpacked_msg;
}