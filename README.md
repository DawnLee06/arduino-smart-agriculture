# Arduino 智慧农业环境监控

基于 Arduino UNO 的农业环境监测与自动控温系统。采集温度/湿度/CO₂ 数据并在 LCD1602 上滚动显示，同时根据温度线性调节 PWM 占空比驱动通风/降温设备（如风扇），实现闭环温控。

## 功能

- **温湿度采集**：DHT11 数字传感器，5 秒采样一次
- **CO₂ 采集**：串口型 CO₂ 模块（0x2C 帧头，`buffer[1]*256 + buffer[2]` 解析）
- **本地显示**：LCD1602 四线驱动，双行滚动展示 CO₂ / 温度 / 湿度
- **自动控温**：温度线性映射 PWM 占空比，26 ℃ 停转、33 ℃ 满载
- **数据上报**：预留 ESP8266 WiFi 上传服务器接口

## 温控逻辑

```c
unsigned int tempMin = 26;   // 零速温度
unsigned int tempMax = 33;   // 满速温度

if (sensor_tem <= tempMin)          dutyCycle = 0;
else if (sensor_tem < tempMax)      dutyCycle = (sensor_tem - tempMin) * 255 / (tempMax - tempMin);
else                                dutyCycle = 255;

analogWrite(10, dutyCycle);          // PWM 输出驱动
```

温度在 26–33 ℃ 区间内做线性插值，PWM 从 0 平滑升到 255，避免启停冲击。

## 硬件接线

| 模块 | 引脚 | 说明 |
|---|---|---|
| DHT11 | D13 | 温湿度数据线 |
| LCD1602 | RS=12, EN=11, D4=5, D5=4, D6=3, D7=2 | 4 位数据线模式 |
| CO₂ 模块 | RX=6, TX=7 | SoftwareSerial 软串口，9600 |
| 风扇/执行器 | D10 | PWM 输出 |
| 调试串口 | USB | 硬件串口，9600 |

> 代码中使用了 `analogReference(INTERNAL)`，如需改为外部基准请注意量程变化。

## 目录结构

```
.
├── Arduinonongye.ino        # 主程序：采样 / 显示 / 控温主循环
├── dht11.cpp / dht11.h      # DHT11 驱动（SimKard / Rob Tillaart 版，GPL v3）
├── library.json             # Arduino 库描述文件
└── examples/
    ├── sketch_mar30a.ino    # 早期实验草图
    ├── sketch_mar30b.ino    # 早期实验草图
    └── sketch_may17f.ino    # 早期实验草图
```

## 依赖库

| 库 | 用途 | 获取方式 |
|---|---|---|
| LiquidCrystal | LCD1602 驱动 | Arduino IDE 内置 |
| SoftwareSerial | 软串口 | Arduino IDE 内置 |
| dht11 | 温湿度传感器 | 本仓库 `dht11.cpp/h`（GPL v3） |
| ESP8266 (WeeESP8266) | WiFi 通讯 | [ITEADLIB_Arduino_WeeESP8266](https://github.com/itead/ITEADLIB_Arduino_WeeESP8266)（GPL v2） |

## 构建与烧录

1. Arduino IDE 中安装 LiquidCrystal 库（内置无需另装）
2. 打开 `Arduinonongye.ino`
3. 开发板选 **Arduino Uno**，端口选对应 COM 口
4. 编译上传，打开串口监视器（9600）查看实时数据

## 待改进

- CO₂ 值目前为 LCD 上的占位显示（`"534 ppm"`），建议接入真实解析值 `CO2_VALUE`
- 主循环中的 LCD 滚动动画用 `delay(1000)` 实现，会阻塞采样；建议改为 `millis()` 非阻塞调度
- ESP8266 上传逻辑尚未接入主循环，可补一个带 `INTERVAL_SENSOR` 节流的发送分支
- 建议把 WiFi SSID / 密码移到独立配置文件并加入 `.gitignore`，避免误提交凭据

## License

主程序逻辑为本人编写。`dht11` 驱动遵循 **GPL v3**，`ESP8266` 驱动遵循 **GPL v2**，使用时请遵守对应协议。
