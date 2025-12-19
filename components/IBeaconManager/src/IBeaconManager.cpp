#include "../include/IBeaconManager.h"
#include <string.h>
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_bt_main.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

// 静态成员初始化
const char *IBeaconManager::TAG = "IBEACON_MANAGER";
const uint8_t IBeaconManager::uuid_zeros[16] = {0};
const IBeaconManager::IBeaconHead IBeaconManager::common_head = {
    .flags = {0x02, 0x01, 0x06},
    .length = 0x1A,
    .type = 0xFF,
    .company_id = 0x004C,
    .beacon_type = 0x1502
};


IBeaconManager *IBeaconManager::instance_ = nullptr;

// 构造函数
IBeaconManager::IBeaconManager(Mode mode) : mode_(mode), is_initialized_(false) {
    const uint8_t default_uuid[16] = {
        0xFD, 0xA5, 0x06, 0x93, 0xA4, 0xE2, 0x4F, 0xB1,
        0xAF, 0xCF, 0xC6, 0xEB, 0x07, 0x64, 0x78, 0x25
    };
    memcpy(vendor_config_.proximity_uuid, default_uuid, 16);
    vendor_config_.major = endianChangeU16(10167);
    vendor_config_.minor = endianChangeU16(61958);
    vendor_config_.measured_power = 0xC5;

    // 初始化回调为空
    found_cb_ = nullptr;
    status_cb_ = nullptr;

    // 保存实例指针（用于静态回调）
    instance_ = this;

    ESP_LOGI(TAG, "IBeacon mode %d", mode_);

    // 初始化蓝牙底层
    initBluetooth();
}

// 析构函数
IBeaconManager::~IBeaconManager() {
    // 此处可添加蓝牙资源释放逻辑（如停止广播/扫描）
}

// 启动功能
esp_err_t IBeaconManager::start() {
    if (!is_initialized_) {
        ESP_LOGE(TAG, "Bluetooth not initialized");
        return ESP_FAIL;
    }

    // 注册GAP回调
    esp_err_t status = esp_ble_gap_register_callback(gapCallback);
    if (status != ESP_OK) {
        ESP_LOGE(TAG, "Gap register failed: %s", esp_err_to_name(status));
        return status;
    }

    // 根据模式配置
    if (mode_ == SENDER) {
        configureSender();
    } else {
        configureReceiver();
    }

    return ESP_OK;
}

// 注册发现设备回调
void IBeaconManager::setIBeaconFoundCallback(IBeaconFoundCallback cb) {
    found_cb_ = cb;
}

// 注册状态回调
void IBeaconManager::setStatusCallback(StatusCallback cb) {
    status_cb_ = cb;
}

// 设置发送参数
void IBeaconManager::setBeaconParams(const uint8_t uuid[16], uint16_t major, uint16_t minor, int8_t measured_power) {
    memcpy(vendor_config_.proximity_uuid, uuid, 16);
    vendor_config_.major = endianChangeU16(major); // 转换为大端
    vendor_config_.minor = endianChangeU16(minor);
    vendor_config_.measured_power = measured_power;
}

// 蓝牙底层初始化
void IBeaconManager::initBluetooth() {
    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 释放经典蓝牙内存
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    // 初始化蓝牙控制器
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));

    // 初始化Bluedroid
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    is_initialized_ = true;
}

// 配置发送模式
void IBeaconManager::configureSender() {
    IBeaconData adv_data;
    esp_err_t status = configIBeaconData(&vendor_config_, &adv_data);
    if (status == ESP_OK) {
        // 配置广播数据并启动广播
        esp_ble_gap_config_adv_data_raw((uint8_t *) &adv_data, sizeof(adv_data));
    } else {
        ESP_LOGE(TAG, "Config iBeacon data failed: %s", esp_err_to_name(status));
    }
}

// 配置接收模式
void IBeaconManager::configureReceiver() {
    esp_ble_scan_params_t scan_params = {
        .scan_type = BLE_SCAN_TYPE_ACTIVE,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
        .scan_interval = 0x50,
        .scan_window = 0x30,
        .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE
    };
    esp_ble_gap_set_scan_params(&scan_params);
}

// 大小端转换
uint16_t IBeaconManager::endianChangeU16(uint16_t x) {
    return (((x & 0xFF00) >> 8) + ((x & 0xFF) << 8));
}

// 判断是否为iBeacon数据包
bool IBeaconManager::isIBeaconPacket(uint8_t *adv_data, uint8_t adv_data_len) {
    if (adv_data == nullptr || adv_data_len != 0x1E) {
        return false;
    }
    return memcmp(adv_data, &common_head, sizeof(IBeaconHead)) == 0;
}

// 配置iBeacon数据
esp_err_t IBeaconManager::configIBeaconData(IBeaconVendor *vendor, IBeaconData *ibeacon_data) {
    if (vendor == nullptr || ibeacon_data == nullptr ||
        memcmp(vendor->proximity_uuid, uuid_zeros, 16) == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    memcpy(&ibeacon_data->head, &common_head, sizeof(IBeaconHead));
    memcpy(&ibeacon_data->vendor, vendor, sizeof(IBeaconVendor));
    return ESP_OK;
}

// GAP事件回调
void IBeaconManager::gapCallback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    if (instance_ == nullptr) return;
    esp_err_t err;
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
            if (instance_->mode_ == SENDER) {
                // 配置广播参数并启动
                esp_ble_adv_params_t adv_params = {
                    .adv_int_min = 0x20,
                    .adv_int_max = 0x40,
                    .adv_type = ADV_TYPE_NONCONN_IND,
                    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
                    .channel_map = ADV_CHNL_ALL,
                    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
                };
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;

        case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
            if (instance_->mode_ == RECEIVER) {
                esp_ble_gap_start_scanning(0); // 0表示持续扫描
            }
            break;

        case ESP_GAP_BLE_SCAN_RESULT_EVT:
            if (instance_->mode_ == RECEIVER) {
                handleScanResult(param);
            }
            break;

        // 状态事件（启动/停止广播/扫描）
        case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            handleStatusEvent(event, param);
            break;

        default:
            break;
    }
}

// 处理扫描结果
void IBeaconManager::handleScanResult(esp_ble_gap_cb_param_t *param) {
    if (param->scan_rst.search_evt != ESP_GAP_SEARCH_INQ_RES_EVT) {
        return;
    }

    if (isIBeaconPacket(param->scan_rst.ble_adv, param->scan_rst.adv_data_len)) {
        IBeaconData *ibeacon = (IBeaconData *) param->scan_rst.ble_adv;
        uint16_t major = endianChangeU16(ibeacon->vendor.major);
        uint16_t minor = endianChangeU16(ibeacon->vendor.minor);

        // 调用用户注册的回调（如果有）
        if (instance_->found_cb_ != nullptr) {
            instance_->found_cb_(
                param->scan_rst.ble_adv,
                param->scan_rst.adv_data_len,
                param->scan_rst.bda,
                ibeacon->vendor.proximity_uuid,
                major,
                minor,
                ibeacon->vendor.measured_power,
                param->scan_rst.rssi
            );
        }
    }
}

// 处理状态事件
void IBeaconManager::handleStatusEvent(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    const char *event_name = "";
    switch (event) {
        case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT: event_name = "Scanning start";
            break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT: event_name = "Advertising start";
            break;
        case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT: event_name = "Scanning stop";
            break;
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT: event_name = "Advertising stop";
            break;
        default: return;
    }

    bool success = (param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS);
    // 调用用户注册的回调（如果有）
    if (instance_->status_cb_ != nullptr) {
        instance_->status_cb_(success, event_name);
    }

    // 日志输出
    if (success) {
        ESP_LOGI(TAG, "%s successfully", event_name);
    } else {
        ESP_LOGE(TAG, "%s failed: %s", event_name, esp_err_to_name(param->adv_start_cmpl.status));
    }
}
