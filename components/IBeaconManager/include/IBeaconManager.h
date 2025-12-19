#ifndef IBEACON_MANAGER_H
#define IBEACON_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"

// 前置声明蓝牙地址类型
typedef uint8_t esp_bd_addr_t[6];

class IBeaconManager {
public:
    // 工作模式枚举
    enum Mode {
        SENDER = 0,    // 发送模式
        RECEIVER = 1   // 接收模式
    };

    // 回调函数类型定义
    // 接收模式：发现iBeacon设备时的回调
    typedef void (*IBeaconFoundCallback)(
        uint8_t *adv_data, // 广播数据
        uint8_t adv_data_len,
        const esp_bd_addr_t addr,          // 设备地址
        const uint8_t uuid[16],            // UUID
        uint16_t major,                    // major值
        uint16_t minor,                    // minor值
        int8_t measured_power,             // 校准功率
        int8_t rssi                        // 信号强度
    );

    // 通用状态回调（广播/扫描启动/停止结果）
    typedef void (*StatusCallback)(
        bool success,      // 操作是否成功
        const char* event  // 事件描述（如"Advertising start"）
    );

    // 构造函数：初始化时指定模式
    explicit IBeaconManager(Mode mode);

    // 析构函数
    ~IBeaconManager();

    // 启动iBeacon功能（根据模式启动发送或接收）
    esp_err_t start();

    // 注册回调函数
    void setIBeaconFoundCallback(IBeaconFoundCallback cb);
    void setStatusCallback(StatusCallback cb);

    // 设置发送模式的iBeacon参数（仅发送模式有效）
    void setBeaconParams(
        const uint8_t uuid[16],
        uint16_t major,
        uint16_t minor,
        int8_t measured_power
    );

private:
    // 嵌套结构体：iBeacon数据格式（替代原esp_ibeacon_api.h定义）
    typedef struct {
        uint8_t flags[3];
        uint8_t length;
        uint8_t type;
        uint16_t company_id;
        uint16_t beacon_type;
    }__attribute__((packed)) IBeaconHead;

    typedef struct {
        uint8_t proximity_uuid[16];
        uint16_t major;
        uint16_t minor;
        int8_t measured_power;
    }__attribute__((packed)) IBeaconVendor;

    typedef struct {
        IBeaconHead head;
        IBeaconVendor vendor;
    }__attribute__((packed)) IBeaconData;

    // 静态成员：日志标签、常量定义
    static const char* TAG;
    static const uint8_t uuid_zeros[16];
    static const IBeaconHead common_head;

    // 成员变量
    Mode mode_;                  // 当前模式
    bool is_initialized_;        // 初始化状态
    IBeaconVendor vendor_config_;// iBeacon参数配置
    IBeaconFoundCallback found_cb_;  // 发现设备回调
    StatusCallback status_cb_;       // 状态回调

    // 静态实例指针（用于GAP回调访问成员）
    static IBeaconManager* instance_;

    // 蓝牙底层初始化
    void initBluetooth();

    // 模式配置
    void configureSender();
    void configureReceiver();

    // 工具函数
    static uint16_t endianChangeU16(uint16_t x);
    static bool isIBeaconPacket(uint8_t* adv_data, uint8_t adv_data_len);
    static esp_err_t configIBeaconData(IBeaconVendor* vendor, IBeaconData* ibeacon_data);

    // GAP事件回调（静态函数）
    static void gapCallback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param);

    // 事件处理
    static void handleScanResult(esp_ble_gap_cb_param_t* param);
    static void handleStatusEvent(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param);
};

#endif // IBEACON_MANAGER_H