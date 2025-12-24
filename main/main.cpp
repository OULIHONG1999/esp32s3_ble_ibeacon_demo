#include "esp_log.h"
#include "IBeaconManager.h"

// 回调函数实现
void onIBeaconFound(uint8_t *dav_data, uint8_t adv_data_len, const esp_bd_addr_t addr, const uint8_t uuid[16],
                    uint16_t major, uint16_t minor, int8_t measured_power, int8_t rssi) {
    // 输出原始数据和长度
    ESP_LOGI("USER", "Adv data(%d): ", adv_data_len);
    for (int i = 0; i < adv_data_len; i++) {
        printf("%02x ", dav_data[i]);
    }
    printf("\n");

    printf("Addr: ");
    // 输出addr
    for (int i = 0; i < 6; i++) {
        printf("%02x ", addr[i]);
    }
    printf("\n");

    ESP_LOGI("USER", "Found iBeacon:");
    ESP_LOGI("USER", "UUID: ");
    for (int i = 0; i < 16; i++) {
        printf("%02x ", uuid[i]);
    }
    printf("\n");
    ESP_LOGI("USER", "Major: %d, Minor: %d, RSSI: %d", major, minor, rssi);
}

void onStatusChanged(bool success, const char *event) {
    ESP_LOGI("USER", "Event: %s, Success: %s", event, success ? "Yes" : "No");
}

extern "C" void app_main() {
    // 初始化接收模式
    static IBeaconManager receiver(IBeaconManager::RECEIVER);
    receiver.setIBeaconFoundCallback(onIBeaconFound);
    receiver.setStatusCallback(onStatusChanged);
    receiver.start();

    // 若需使用发送模式：
    // static IBeaconManager sender(IBeaconManager::SENDER);
    // uint8_t my_uuid[16] = {
    //     0xfd, 0xa5, 0x06, 0x93, 0xa4, 0xe2, 0x4f, 0xb1, 0xaf, 0xcf, 0x6e, 0xb0, 0x76, 0x47, 0x82, 0x05
    // }; // 自定义UUID
    // sender.setBeaconParams(my_uuid, 1, 2, -59); // 设置参数
    // sender.setStatusCallback(onStatusChanged);
    // sender.start();
}
