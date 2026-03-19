# BeShell-MG Example App

这是一个演示 BeShell-MG (Mongoose 网络库) 用法的示例工程。

## 功能特性

本示例工程演示了以下功能：

### 网络功能 (MG Module)

| 示例文件 | 说明 |
|---------|------|
| `http-server.js` | 创建 HTTP Web 服务器，支持路由处理 |
| `http-client.js` | 发送 HTTP GET/POST 请求 |
| `http-download.js` | 下载文件，支持进度显示 |
| `mqtt-client.js` | 连接 MQTT Broker，订阅和发布消息 |
| `websocket-server.js` | 创建 WebSocket 服务器，支持双向通信 |
| `tcp-client.js` | 原始 TCP 套接字通信 |
| `sntp-time.js` | 网络时间同步，DNS 缓存操作 |

### WiFi 功能

| 示例文件 | 说明 |
|---------|------|
| `wifi-sta.js` | 连接到 WiFi 热点 (Station 模式) |
| `wifi-ap.js` | 创建 WiFi 热点 (AP 模式) |
| `wifi-scan.js` | 扫描周围的 WiFi 网络 |

### 基础功能

| 示例文件 | 说明 |
|---------|------|
| `gpio-blink.js` | GPIO 控制，LED 闪烁 |

## 项目结构

```
beshell-mg-app/
├── CMakeLists.txt          # 项目 CMake 配置
├── sdkconfig.defaults      # 默认 SDK 配置
├── README.md               # 本文件
├── main/
│   ├── main.cpp            # C++ 入口文件
│   ├── CMakeLists.txt      # main 组件 CMake 配置
│   └── idf_component.yml   # 组件依赖配置
├── img/
│   ├── partitions-4MB.csv  # 4MB Flash 分区表
│   ├── partitions-8MB.csv  # 8MB Flash 分区表
│   └── partitions-16MB.csv # 16MB Flash 分区表
└── js/
    ├── main.js             # JS 入口文件
    └── example/            # 示例脚本目录
        ├── http-server.js
        ├── http-client.js
        ├── http-download.js
        ├── mqtt-client.js
        ├── websocket-server.js
        ├── tcp-client.js
        ├── sntp-time.js
        ├── wifi-sta.js
        ├── wifi-ap.js
        ├── wifi-scan.js
        └── gpio-blink.js
```

## 硬件要求

- ESP32 / ESP32-S3 / ESP32-C3 等支持 WiFi 的芯片
- 至少 4MB Flash
- USB 转串口用于烧录和调试

## 快速开始

### 1. 配置 WiFi 凭证

编辑 `js/example/wifi-sta.js`、`js/example/http-server.js` 等文件，修改 WiFi SSID 和密码：

```javascript
const ssid = "your SSID"
const pwd = "your PASSWORD"
```

### 2. 构建项目

```bash
idf.py build
```

### 3. 烧录固件

```bash
idf.py flash
```

### 4. 查看串口输出

```bash
idf.py monitor
```

### 5. 运行示例

在串口终端中，输入以下命令运行示例：

```
run /example/http-server.js
```

## 示例详解

### HTTP Server

创建一个简单的 HTTP Web 服务器：

```javascript
import * as mg from "mg"

mg.listenHttp("0.0.0.0:8080", (event, req, rspn) => {
    if (event === "http.msg") {
        rspn.setStatus(200)
        rspn.setHeader("Content-Type", "text/html")
        rspn.send("<h1>Hello World</h1>")
    }
})
```

### HTTP Client

发送 HTTP GET 请求：

```javascript
import * as mg from "mg"

let body = await mg.get("http://api.example.com/data")
console.log(body.toString())
```

### HTTP Download

下载文件并显示进度：

```javascript
import * as mg from "mg"

// Download to memory
await mg.download("http://example.com/file.bin", null, (total, current, chunk) => {
    console.log(`Progress: ${(current/total*100).toFixed(1)}%`)
})

// Download to file
await mg.download("http://example.com/file.bin", "/data/file.bin", (total, current) => {
    console.log(`Downloaded: ${current}/${total} bytes`)
})
```

### MQTT Client

连接 MQTT Broker：

```javascript
import * as mg from "mg"

let client = mg.connect("mqtt://broker.emqx.io:1883", (event, data) => {
    if (event === "mqtt.open") {
        client.sub("test/topic")
        client.push("test/topic", "Hello MQTT")
    }
    else if (event === "mqtt.msg") {
        console.log("Received:", data.body().toString())
    }
})
```

### WebSocket Server

创建 WebSocket 服务器：

```javascript
import * as mg from "mg"

mg.listenHttp("0.0.0.0:8080", (event, req, rspn) => {
    if (event === "ws.open") {
        console.log("Client connected")
    }
    else if (event === "ws.msg") {
        let msg = req.body().toString()
        req.send("Echo: " + msg)
    }
})
```

### TCP Client

原始 TCP 套接字通信：

```javascript
import * as mg from "mg"

let conn = mg.connect("tcp://example.com:1234", (event, data) => {
    if (event === "connect") {
        conn.send("Hello TCP Server")
    }
    else if (event === "read") {
        console.log("Received:", data.body().toString())
    }
})
```

### SNTP Time

同步网络时间：

```javascript
import * as mg from "mg"

mg.sntpRequest("udp://pool.ntp.org:123", (err, timestamp) => {
    if (!err) {
        let date = new Date(timestamp)
        console.log("Current time:", date.toISOString())
    }
})
```

## 依赖组件

- [beshell](https://github.com/become-cool/beshell) - BeShell 核心框架
- [beshell-mg](https://github.com/become-cool/beshell) - Mongoose 网络库封装

## 许可证

LGPL

## 相关链接

- [BeShell 文档](https://beshell.become.cool)
- [Mongoose 文档](https://mongoose.ws/documentation/)
- [ESP-IDF 文档](https://docs.espressif.com/projects/esp-idf/)
