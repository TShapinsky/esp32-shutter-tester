typedef struct __message_measure_t {
    float duration;
} message_measure_t;

#define MSG_ID_MEASURE_LEN 4
#define MSG_ID_MEASURE 0x30

static inline message_t msg_measure_pack(message_measure_t msg) {
    uint8_t buf[MSG_ID_MEASURE_LEN];
    message_t packed_msg = {
        .len = MSG_ID_MEASURE_LEN,
        .type = MSG_ID_MEASURE,
        .body = buf,
    };
    memcpy(buf, &(msg.duration), sizeof(msg.duration));

    return packed_msg;
}

static inline message_measure_t msg_measure_unpack(message_t msg) {
    message_measure_t unpacked_msg = {
        .duration = *((float *)msg.body),
    };
    return unpacked_msg;
}