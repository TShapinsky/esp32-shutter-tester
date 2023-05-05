#define MSG_MAX_PAYLOAD_LEN 1024
#define MSG_HEADER_SIZE sizeof(uint8_t) + sizeof(uint32_t)

typedef struct __message_t{
    uint8_t type;
    uint32_t len;
    uint8_t * body;
} message_t;

void inline send_msg(uint32_t handle, message_t msg) {
    uint8_t buf[MSG_HEADER_SIZE + msg.len];
    uint8_t * idx = buf;

    memcpy(idx, &(msg.type), sizeof(uint8_t)); idx += sizeof(uint8_t);
    memcpy(idx, &(msg.len), sizeof(uint32_t)); idx += sizeof(uint32_t);
    memcpy(idx, msg.body, msg.len);
    // for (int i = 0; i < msg.len; i++) {
    //     ESP_LOGI("MESSAGE_SEND", "%x", idx[i]);
    // }
    esp_spp_write(handle, MSG_HEADER_SIZE + msg.len, buf);
}