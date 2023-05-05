
typedef struct __message_heartbeat_t {
    uint32_t echo; 
} message_heartbeat_t;

#define MSG_ID_HEARTBEAT_LEN 4
#define MSG_ID_HEARTBEAT 0x01

static inline message_t msg_heartbeat_pack(message_heartbeat_t msg){
    uint8_t buf[MSG_ID_HEARTBEAT_LEN];
    message_t packed_msg = {
        .len = MSG_ID_HEARTBEAT_LEN,
        .type = MSG_ID_HEARTBEAT,
        .body = buf,
    };
    memcpy(buf, &(msg.echo), sizeof(msg.echo));

    return packed_msg;
}

static inline message_heartbeat_t msg_heartbeat_unpack(message_t msg) {
    message_heartbeat_t unpacked_msg = {
        .echo = *((uint32_t *)msg.body),
    };
    ESP_LOGI("HEARTBEAT", "Unpacked Echo: %d", unpacked_msg.echo);
    return unpacked_msg;
}