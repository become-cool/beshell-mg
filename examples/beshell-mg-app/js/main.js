import * as fs from "fs"

let examples = fs.listDirSync("/example")
  .sort((a,b)=>a.localeCompare(b))
  .reduce((lst,filename)=>{
      return lst + `    run /example/${filename}\n`
  },'')


console.log(`
  ============================================
   BeShell-MG (Mongoose Network) Example App
  ============================================

  This is a demo for BeShell-MG networking features:
  
  * HTTP Server    - Create web server on ESP32
  * HTTP Client    - Send HTTP requests
  * HTTP Download  - Download files with progress
  * MQTT Client    - Connect to MQTT broker
  * WebSocket      - Real-time bidirectional communication
  * TCP Client     - Raw TCP socket communication
  * SNTP           - Network time synchronization
  * WiFi STA/AP    - WiFi connection modes
  * WiFi Scan      - Scan for networks

  Available Examples:
  ${examples}
  
  Commands:
  * Enter \`ls /example\` to list all examples
  * Enter \`run <full example path>\` to run example
  * Enter \`reboot\` to restart
  * Enter \`help\` or \`?\` to list all commands
  * Enter JavaScript code to run in interactive mode
`)

console.log(`
  ============================================
   BeShell-MG (Mongoose 网络库) 示例程序
  ============================================

  本示例演示 BeShell-MG 的网络功能：
  
  * HTTP Server    - 在 ESP32 上创建 Web 服务器
  * HTTP Client    - 发送 HTTP 请求
  * HTTP Download  - 带进度下载文件
  * MQTT Client    - 连接 MQTT 服务器
  * WebSocket      - 实时双向通信
  * TCP Client     - 原始 TCP 套接字通信
  * SNTP           - 网络时间同步
  * WiFi STA/AP    - WiFi 连接模式
  * WiFi Scan      - 扫描网络

  可用示例：
  ${examples}
  
  命令：
  * 输入 \`ls /example\` 列出所有示例
  * 输入 \`run <完整路径>\` 运行示例
  * 输入 \`reboot\` 重启
  * 输入 \`help\` 或 \`?\` 列出所有命令
  * 输入 JavaScript 代码进入交互模式
`)
