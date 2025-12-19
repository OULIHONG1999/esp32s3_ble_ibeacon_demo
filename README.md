| 支持的芯片 | ESP32 | ESP32-S3 |
| ---------- | ----- | -------- |

# ESP-IDF iBeacon 扫描器

本项目演示了使用 ESP32/ESP32-S3 进行 iBeacon 扫描的功能。项目使用自定义的 IBeaconManager 组件来简化 iBeacon 接收功能的实现。

## 如何使用示例

在配置和构建项目之前，请确保使用以下命令设置正确的芯片目标：

```bash
idf.py set-target <chip_name>
```

### 硬件要求

* 一块 ESP32/ESP32-S3 开发板
* 一个或多个 iBeacon 发射器

### 配置项目

打开项目配置菜单：

```bash
idf.py menuconfig
```

### 构建和烧录

构建项目并将其烧录到您的 ESP32 开发板：

```bash
idf.py -p PORT flash monitor
```

（要退出串口监视器，请按 ``Ctrl-]``）

有关配置和使用 ESP-IDF 构建项目的完整步骤，请参阅[入门指南](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html)。

## 示例输出

当扫描器检测到 iBeacon 设备时，您将看到如下输出：

```
I (475) USER: Event: Scanning start, Success: Yes
I (475) IBEACON_MANAGER: Scanning start successfully
I (1165) USER: Adv data(30): 
02 01 06 1a ff 4c 00 02 15 fd a5 06 93 a4 e2 4f b1 af cf 6e b0 76 47 82 05 01 00 02 00 d8
I (1165) USER: Found iBeacon:
I (1165) USER: UUID: 
fd a5 06 93 a4 e2 4f b1 af cf 6e b0 76 47 82 05
I (1175) USER: Major: 256, Minor: 512, RSSI: -75
```

## 项目结构

本项目使用位于 `components` 目录下的自定义 IBeaconManager 组件：
- `components/IBeaconManager/include/IBeaconManager.h` - 类定义头文件
- `components/IBeaconManager/src/IBeaconManager.cpp` - 实现文件
- `components/IBeaconManager/example/iBeacon_scan` - 组件使用示例

## 故障排除

如有任何技术问题，请在 GitHub 上提交 [issue](https://github.com/espressif/esp-idf/issues)，我们会尽快回复您。
