# BeShell-MG Example App

This is an example project demonstrating the usage of BeShell-MG (Mongoose networking library).

## Features

This example project demonstrates the following features:

### Networking (MG Module)

| Example File | Description |
|---------|------|
| `http-server.js` | Create HTTP Web server with routing |
| `http-client.js` | Send HTTP GET/POST requests |
| `http-download.js` | Download files with progress display |
| `mqtt-client.js` | Connect to MQTT broker, subscribe and publish |
| `websocket-server.js` | Create WebSocket server with bidirectional communication |
| `tcp-client.js` | Raw TCP socket communication |
| `sntp-time.js` | Network time synchronization, DNS cache operations |

### WiFi Features

| Example File | Description |
|---------|------|
| `wifi-sta.js` | Connect to WiFi hotspot (Station mode) |
| `wifi-ap.js` | Create WiFi hotspot (AP mode) |
| `wifi-scan.js` | Scan for nearby WiFi networks |

### Basic Features

| Example File | Description |
|---------|------|
| `gpio-blink.js` | GPIO control, LED blinking |

## Project Structure

```
beshell-mg-app/
├── CMakeLists.txt          # Project CMake configuration
├── sdkconfig.defaults      # Default SDK configuration
├── README.md               # This file
├── main/
│   ├── main.cpp            # C++ entry file
│   ├── CMakeLists.txt      # main component CMake config
│   └── idf_component.yml   # Component dependencies
├── img/
│   ├── partitions-4MB.csv  # 4MB Flash partition table
│   ├── partitions-8MB.csv  # 8MB Flash partition table
│   └── partitions-16MB.csv # 16MB Flash partition table
└── js/
    ├── main.js             # JS entry file
    └── example/            # Example scripts directory
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

## Hardware Requirements

- ESP32 / ESP32-S3 / ESP32-C3 or other WiFi-capable chips
- At least 4MB Flash
- USB-to-Serial for flashing and debugging

## Quick Start

### 1. Configure WiFi Credentials

Edit `js/example/wifi-sta.js`, `js/example/http-server.js`, etc., to set your WiFi SSID and password:

```javascript
const ssid = "your SSID"
const pwd = "your PASSWORD"
```

### 2. Build the Project

```bash
idf.py build
```

### 3. Flash the Firmware

```bash
idf.py flash
```

### 4. View Serial Output

```bash
idf.py monitor
```

### 5. Run Examples

In the serial terminal, enter the following command to run an example:

```
run /example/http-server.js
```

## Example Details

### HTTP Server

Create a simple HTTP Web server:

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

Send HTTP GET request:

```javascript
import * as mg from "mg"

let body = await mg.get("http://api.example.com/data")
console.log(body.toString())
```

### HTTP Download

Download file with progress:

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

Connect to MQTT Broker:

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

Create WebSocket server:

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

Raw TCP socket communication:

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

Synchronize network time:

```javascript
import * as mg from "mg"

mg.sntpRequest("udp://pool.ntp.org:123", (err, timestamp) => {
    if (!err) {
        let date = new Date(timestamp)
        console.log("Current time:", date.toISOString())
    }
})
```

## Dependencies

- [beshell](https://github.com/become-cool/beshell) - BeShell core framework
- [beshell-mg](https://github.com/become-cool/beshell) - Mongoose networking library wrapper

## License

LGPL

## Links

- [BeShell Documentation](https://beshell.become.cool)
- [Mongoose Documentation](https://mongoose.ws/documentation/)
- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/)
