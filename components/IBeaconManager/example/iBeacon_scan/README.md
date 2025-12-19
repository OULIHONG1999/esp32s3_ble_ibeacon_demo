## ESP-IDF iBeacon 扫描示例

本示例演示了如何使用 ESP32 作为 iBeacon 接收器来扫描和解析周围的 iBeacon 设备。

## 功能介绍

此示例将 ESP32 配置为 iBeacon 接收器，能够：
- 主动扫描周围的蓝牙低功耗设备
- 识别并解析符合 iBeacon 格式的广播数据包
- 提取 iBeacon 的关键信息，包括 UUID、Major、Minor 和 RSSI 值
- 通过回调函数通知应用程序发现的 iBeacon 设备

## 如何使用示例

### 硬件要求

- 一个 ESP32 开发板（如 ESP32-DevKitC）
- 一个或多个 iBeacon 发射器设备（或其他设备配置为 iBeacon 发射器）

### 配置项目

1. 设置目标芯片：

```bash
idf.py set-target esp32s3
```


2. （可选）使用以下命令配置项目：

```bash
idf.py menuconfig
```


在菜单中可以调整蓝牙相关配置，例如扫描参数等。

### 构建和烧录

1. 构建、烧录并监视项目：

```bash
idf.py -p PORT flash monitor
```


（要退出串口监视器，请按 ```Ctrl-
]``）

有关配置和使用 ESP-IDF 构建项目的完整步骤，请参阅 [入门指南](https://idf.espressif.com/)。

## 示例工作原理

### 初始化过程

1. 创建静态 [IBeaconManager](file:///E:/WORK/ESP/project/esp32s3_beacon_scan/components/IBeaconManager/include/IBeaconManager.h#L15-L117) 实例并设置为接收模式
2. 注册回调函数以处理发现的 iBeacon 和状态变化
3. 调用 [start()](file:///E:/WORK/ESP/project/esp32s3_beacon_scan/components/IBeaconManager/include/IBeaconManager.h#L46-L46) 方法启动蓝牙并开始扫描

### 事件处理

当扫描到 iBeacon 设备时，会触发以下流程：

1. 蓝牙控制器检测到广播数据包
2. [handleScanResult](file:///E:/WORK/ESP/project/esp32s3_beacon_scan/components/IBeaconManager/src/IBeaconManager.cpp#L238-L266) 函数验证数据包是否符合 iBeacon 格式
3. 如果匹配，则提取 UUID、Major、Minor 和 RSSI 等信息
4. 调用用户注册的 [onIBeaconFound](file:///E:/WORK/ESP/project/esp32s3_beacon_scan/components/IBeaconManager/example/iBeacon_scan/main/main.cpp#L5-L18) 回调函数处理数据

### 回调函数

示例实现了两个主要回调函数：

1. [onIBeaconFound](file:///E:/WORK/ESP/project/esp32s3_beacon_scan/components/IBeaconManager/example/iBeacon_scan/main/main.cpp#L5-L18) - 当发现新的 iBeacon 设备时调用
2. [onStatusChanged](file:///E:/WORK/ESP/project/esp32s3_beacon_scan/components/IBeaconManager/example/iBeacon_scan/main/main.cpp#L20-L22) - 当扫描状态改变时调用

## 示例输出

当成功扫描到 iBeacon 设备时，您将在串口监视器中看到类似以下的输出：

```
I (1165) USER: Adv data(30): 
02 01 06 1a ff 4c 00 02 15 fd a5 06 93 a4 e2 4f b1 af cf 6e b0 76 47 82 05 01 00 02 00 d8
I (1165) USER: Found iBeacon:
I (1165) USER: UUID: 
fd a5 06 93 a4 e2 4f b1 af cf 6e b0 76 47 82 05
I (1175) USER: Major: 256, Minor: 512, RSSI: -75
```

这表明系统已经成功识别了一个 iBeacon 设备，并提取了其广播数据中的关键信息。

## 故障排除

如果遇到问题，请检查以下几点：

1. 确保您的 iBeacon 设备正在正常广播
2. 检查 ESP32s3 的天线连接是否良好
3. 确认电源供应充足
4. 检查串口监视器输出以获取错误信息

对于任何技术问题，请在 GitHub 上提交 [issue](https://github.com/espressif/esp-idf/issues)，我们将尽快回复您。```
