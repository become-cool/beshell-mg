import * as mg from "mg"
import * as wifi from "wifi"

const ssid = "your SSID"
const pwd = "your PASSWORD"

// TCP Echo Server for testing
// You can use: nc -l 12345 (on Linux/Mac)
// Or use online TCP echo servers
const TCP_SERVER = "tcp://tcpbin.com:4242"

async function main() {
    console.log("\n=== TCP Client Example ===\n")
    
    // 1. Connect to WiFi
    console.log("Connecting to WiFi...")
    if (!await wifi.connect(ssid, pwd)) {
        console.log("Failed to connect to WiFi")
        return
    }
    
    let status = await wifi.waitIP()
    if (!status) {
        console.log("Failed to get IP")
        return
    }
    
    console.log("WiFi connected, IP:", status.ip)
    console.log("")
    
    // 2. Connect to TCP server
    console.log("Connecting to TCP server:", TCP_SERVER)
    
    try {
        let conn = mg.connect(TCP_SERVER, (event, data) => {
            console.log("TCP Event:", event)
            
            if (event === "connect") {
                console.log("Connected to TCP server!")
                
                // Send data
                let message = "Hello from BeShell-MG TCP Client!\n"
                console.log("Sending:", message.trim())
                conn.send(message)
            }
            else if (event === "read") {
                // Received data
                let received = data.body()
                console.log("Received:", received.toString())
            }
            else if (event === "error") {
                console.log("TCP Error:", data)
            }
            else if (event === "close") {
                console.log("TCP connection closed")
            }
        })
        
        // Send periodic messages
        let counter = 1
        let interval = setInterval(() => {
            try {
                let msg = `Message #${counter} from BeShell-MG\n`
                conn.send(msg)
                counter++
                
                if (counter > 5) {
                    clearInterval(interval)
                    console.log("\nClosing connection...")
                    conn.close()
                }
            } catch (e) {
                console.log("Send error:", e)
                clearInterval(interval)
            }
        }, 2000)
        
    } catch (e) {
        console.log("Failed to connect:", e)
    }
}

main()
