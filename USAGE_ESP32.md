# ESP32 使用说明

这个版本适配经典 ESP32-D0WD-V3 / ESP32 DevKit 类开发板。

## 功能

- ESP32 启动一个配置热点 `WIFI MANAGER`。
- 手机或电脑连接后打开 `http://192.168.4.1`。
- 填入需要模拟的 Wi-Fi `SSID`、密码和 AP MAC/BSSID。
- 保存后，ESP32 会创建一个 2.4GHz 热点，并使用填写的 SSID 和 BSSID。
- 模拟热点不会主动下发默认路由，手机连接后应继续优先使用蜂窝数据访问互联网。

## 注意

- 这里的 MAC 是目标 Wi-Fi 的 AP MAC/BSSID，不是 Mac 电脑自己的网卡 MAC。
- ESP32 只能创建 2.4GHz 热点，不能模拟 5GHz、802.11ax 或原信道。
- BSSID 必须是合法单播 MAC，首字节最低位不能为 1。
- 如果原 Wi-Fi 还在附近广播同一个 BSSID，客户端可能出现连接不稳定或识别混乱。
- 手机端请保持 `IP 设置` 为 DHCP，不要手动设置静态 IP。
- iOS/Android 是否保留蜂窝数据由系统策略决定；这个固件会尽量避免把 ESP32 声明成互联网出口。

## 本机适配

- PlatformIO 环境：`esp32dev`
- 默认 LED 引脚：`GPIO2`
- 默认清除配置按键：`GPIO0`

长按 `GPIO0` 超过 1 秒会清除保存的 Wi-Fi 信息并重启。很多 ESP32 开发板的 BOOT 键就是 `GPIO0`。

## 重新编译/上传

```bash
pio run -e esp32dev
pio run -e esp32dev -t upload
pio run -e esp32dev -t uploadfs
```

如果修改了 `data/` 下的网页文件，需要重新执行 `uploadfs`。
