# IBeaconManager 组件

IBeaconManager 是一个用于 ESP-IDF 的组件，它简化了 ESP32 设备上 iBeacon 协议的发送和接收操作。

## 功能特点

- 支持 iBeacon 发送和接收两种模式
- 封装了蓝牙初始化和配置细节
- 提供简单的 API 接口和回调机制
- 易于集成到现有项目中

## 使用方法

### 添加组件

将 IBeaconManager 组件复制到项目的 `components` 目录下，ESP-IDF 构建系统会自动发现并链接该组件。

### 在项目中包含头文件

```cpp
#include "IBeaconManager.h"
```

### 初始化和使用

#### 接收模式

```cpp
// 创建接收器实例
static IBeaconManager receiver(IBeaconManager::RECEIVER);

// 设置回调函数
receiver.setIBeaconFoundCallback(onIBeaconFound);
receiver.setStatusCallback(onStatusChanged);

// 启动
receiver.start();
```

#### 发送模式

```cpp
// 创建发送器实例
IBeaconManager sender(IBeaconManager::SENDER);

// 设置 iBeacon 参数
uint8_t my_uuid[16] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10};
sender.setBeaconParams(my_uuid, 123, 456, -59);

// 设置回调函数
sender.setStatusCallback(onStatusChanged);

// 启动
sender.start();
```

### 回调函数

#### IBeacon 发现回调

```cpp
void onIBeaconFound(uint8_t *adv_data, uint8_t adv_data_len, const esp_bd_addr_t addr, const uint8_t uuid[16],
                    uint16_t major, uint16_t minor, int8_t measured_power, int8_t rssi) {
    // 处理发现的 iBeacon 设备
}
```

#### 状态变更回调

```cpp
void onStatusChanged(bool success, const char *event) {
    // 处理状态变更事件
}
```

## API 参考

### 构造函数

```cpp
IBeaconManager(Mode mode);
```

- `mode`: 工作模式，可以是 `IBeaconManager::SENDER` 或 `IBeaconManager::RECEIVER`

### 启动方法

```cpp
esp_err_t start();
```

启动 iBeacon 功能，根据初始化时设置的模式进行相应配置。

### 回调设置

```cpp
void setIBeaconFoundCallback(IBeaconFoundCallback cb);
void setStatusCallback(StatusCallback cb);
```

- `setIBeaconFoundCallback`: 设置发现 iBeacon 设备时的回调函数（仅接收模式）
- `setStatusCallback`: 设置状态变更回调函数

### 发送参数设置

```cpp
void setBeaconParams(const uint8_t uuid[16], uint16_t major, uint16_t minor, int8_t measured_power);
```

设置 iBeacon 广播参数（仅发送模式有效）。

## 示例

组件包含一个示例项目，位于 `example/iBeacon_scan` 目录下，演示了如何使用 IBeaconManager 接收和解析 iBeacon 广播数据。

可以通过以下步骤运行示例：

1. 进入示例目录
2. 配置目标平台：`idf.py set-target esp32`
3. 编译和烧录：`idf.py build flash monitor`

## 注意事项

1. 使用前请确保目标平台支持蓝牙功能
2. iBeacon 发送模式需要正确设置 UUID 和功率参数
3. 在接收模式下，可以通过调整扫描参数来优化性能