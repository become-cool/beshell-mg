import * as mg from "mg"
import * as wifi from "wifi"

const ssid = "your SSID"
const pwd = "your PASSWORD"
const PORT = 8080

async function main() {
    console.log("\n=== WebSocket Server Example ===\n")
    
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
    console.log("WebSocket Server will run at ws://" + status.ip + ":" + PORT)
    console.log("\nYou can test with a WebSocket client at:")
    console.log("  ws://" + status.ip + ":" + PORT + "/ws")
    console.log("\nPress Ctrl+C to stop\n")
    
    // Track connected WebSocket clients
    let clients = []
    
    // 2. Create HTTP/WebSocket Server
    mg.listenHttp("0.0.0.0:" + PORT, (event, req, rspn) => {
        
        // Handle WebSocket events
        if (event === "ws.open") {
            console.log("WebSocket client connected")
            clients.push(req)
            
            // Send welcome message
            req.send("Welcome to BeShell-MG WebSocket Server!")
            req.send("Connected clients: " + clients.length)
        }
        else if (event === "ws.msg") {
            let message = req.body()
            console.log("WebSocket message received:", message.toString())
            
            // Echo the message back
            req.send("Echo: " + message.toString())
            
            // Broadcast to all clients
            let broadcastMsg = "Broadcast [" + clients.length + " clients]: " + message.toString()
            clients.forEach(client => {
                try {
                    client.send(broadcastMsg)
                } catch (e) {
                    // Client might be disconnected
                }
            })
        }
        else if (event === "ws.ctl") {
            console.log("WebSocket control frame received")
        }
        else if (event === "close") {
            console.log("WebSocket client disconnected")
            // Remove from clients list
            let idx = clients.indexOf(req)
            if (idx > -1) {
                clients.splice(idx, 1)
            }
        }
        
        // Handle regular HTTP requests
        else if (event === "http.msg") {
            let url = req.url()
            console.log("HTTP Request:", req.method(), url)
            
            if (url === "/") {
                rspn.setStatus(200)
                rspn.setHeader("Content-Type", "text/html")
                rspn.send(`
                    <html>
                    <head><title>BeShell-MG WebSocket Server</title></head>
                    <body>
                        <h1>WebSocket Server Demo</h1>
                        <p>Server is running at ws://${status.ip}:${PORT}/ws</p>
                        <div id="log" style="border:1px solid #ccc; padding:10px; height:200px; overflow:auto;"></div>
                        <br>
                        <input type="text" id="msg" placeholder="Enter message">
                        <button onclick="send()">Send</button>
                        <script>
                            var ws = new WebSocket('ws://${status.ip}:${PORT}/ws');
                            var log = document.getElementById('log');
                            
                            ws.onopen = function() {
                                log.innerHTML += '<p>Connected!</p>';
                            };
                            ws.onmessage = function(e) {
                                log.innerHTML += '<p>Received: ' + e.data + '</p>';
                                log.scrollTop = log.scrollHeight;
                            };
                            ws.onclose = function() {
                                log.innerHTML += '<p>Disconnected!</p>';
                            };
                            
                            function send() {
                                var input = document.getElementById('msg');
                                ws.send(input.value);
                                input.value = '';
                            }
                        </script>
                    </body>
                    </html>
                `)
            }
            else if (url === "/ws") {
                // WebSocket upgrade happens automatically
                rspn.setStatus(426)
                rspn.setHeader("Content-Type", "text/plain")
                rspn.send("Upgrade Required - Use WebSocket protocol")
            }
            else {
                rspn.setStatus(404)
                rspn.setHeader("Content-Type", "text/plain")
                rspn.send("404 Not Found")
            }
        }
    })
    
    // Send periodic heartbeat to all clients
    setInterval(() => {
        let heartbeat = "Heartbeat: " + new Date().toISOString()
        clients.forEach(client => {
            try {
                client.send(heartbeat)
            } catch (e) {
                // Client might be disconnected
            }
        })
    }, 10000)
}

main()
