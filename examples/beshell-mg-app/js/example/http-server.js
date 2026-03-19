import * as mg from "mg"
import * as wifi from "wifi"

const ssid = "your SSID"
const pwd = "your PASSWORD"
const PORT = 8080

async function main() {
    console.log("\n=== HTTP Server Example ===\n")
    
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
    console.log("HTTP Server will run at http://" + status.ip + ":" + PORT)
    console.log("\nTry these URLs in your browser:")
    console.log("  http://" + status.ip + ":" + PORT + "/")
    console.log("  http://" + status.ip + ":" + PORT + "/hello")
    console.log("  http://" + status.ip + ":" + PORT + "/json")
    console.log("\nPress Ctrl+C to stop\n")
    
    // 2. Create HTTP Server
    mg.listenHttp("0.0.0.0:" + PORT, (event, req, rspn) => {
        console.log("Event:", event)
        
        if (event === "http.msg") {
            let url = req.url()
            console.log("Request:", req.method(), url)
            
            // Route handling
            if (url === "/") {
                rspn.setStatus(200)
                rspn.setHeader("Content-Type", "text/html")
                rspn.send(`
                    <html>
                    <head><title>BeShell-MG HTTP Server</title></head>
                    <body>
                        <h1>Hello from BeShell-MG!</h1>
                        <p>This is a simple HTTP server running on ESP32.</p>
                        <ul>
                            <li><a href="/hello">/hello</a> - Say hello</li>
                            <li><a href="/json">/json</a> - Get JSON data</li>
                        </ul>
                    </body>
                    </html>
                `)
            }
            else if (url === "/hello") {
                rspn.setStatus(200)
                rspn.setHeader("Content-Type", "text/plain")
                rspn.send("Hello, World! From BeShell-MG HTTP Server.")
            }
            else if (url === "/json") {
                rspn.setStatus(200)
                rspn.setHeader("Content-Type", "application/json")
                let data = {
                    device: "ESP32",
                    platform: "BeShell-MG",
                    uptime: Date.now(),
                    message: "Hello from JSON API"
                }
                rspn.send(JSON.stringify(data))
            }
            else {
                rspn.setStatus(404)
                rspn.setHeader("Content-Type", "text/plain")
                rspn.send("404 Not Found")
            }
        }
    })
}

main()
