# dingding_develop

这是一个基于 ESP32 的 Wi-Fi 标识模拟工具，用于在受控测试环境中创建指定名称和指定 BSSID/MAC 的 2.4GHz 热点。它适合把 ESP32 作为“Wi-Fi 环境标识”使用，而不是作为真正的上网路由器。

项目基于 DingDing WiFi 的思路改造，已适配经典 ESP32 开发板，例如 `esp32dev`。

## 功能

- 通过网页配置需要模拟的 `SSID`、密码和 AP MAC/BSSID。
- 配置会保存到 ESP32 的 NVS，断电重启后可直接进入模拟热点模式。
- 支持修改 SoftAP 的 AP MAC/BSSID。
- 校验 MAC 地址格式，并拒绝非法的组播 MAC。
- 使用 LittleFS 存放配置网页。
- 模拟热点不会主动声明自己是互联网网关，便于手机在连接该 Wi-Fi 后继续使用蜂窝数据。
- 长按配置按键可清空已保存的 Wi-Fi 信息并回到配置模式。

## 硬件

- 经典 ESP32 开发板，例如 ESP32 DevKit。
- 默认 LED 引脚：`GPIO2`。
- 默认清除配置按键：`GPIO0`，很多开发板上的 BOOT 键就是这个引脚。

注意：经典 ESP32 只支持 2.4GHz Wi-Fi，不能模拟 5GHz、Wi-Fi 6/802.11ax、真实信道宽度或路由转发能力。

## 编译与烧录

先安装 PlatformIO，然后执行：

```bash
pio run -e esp32dev
pio run -e esp32dev -t upload
pio run -e esp32dev -t uploadfs
```

如果 PlatformIO 没有自动识别串口，可以手动指定：

```bash
pio run -e esp32dev -t upload --upload-port <serial-port>
pio run -e esp32dev -t uploadfs --upload-port <serial-port>
```

修改 `data/` 目录下的网页文件后，需要重新执行 `uploadfs`。

## 使用方法

1. 给 ESP32 上电。
2. 连接默认热点 `WIFI MANAGER`。
3. 打开 `http://192.168.4.1`。
4. 填入目标 Wi-Fi 的名称、密码和 AP MAC/BSSID。
5. 提交后，ESP32 会使用保存的配置重新创建模拟热点。

这里的 MAC/BSSID 指的是目标 AP 的 MAC 地址，不是手机或电脑自己的 Wi-Fi MAC 地址。

## 清空配置

长按 `GPIO0` 超过 1 秒会清除已保存的 SSID、密码和 MAC/BSSID，然后自动重启回到 `WIFI MANAGER` 配置模式。

## 隐私说明

仓库中不包含任何已保存的 SSID、密码或 BSSID。运行时配置只会在设备端通过网页提交后保存到 ESP32 的 NVS 中。
