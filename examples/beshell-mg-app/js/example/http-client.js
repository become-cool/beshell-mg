import * as mg from "mg"
import * as wifi from "wifi"

const ssid = "your SSID"
const pwd = "your PASSWORD"

async function main() {
    console.log("\n=== HTTP Client Example ===\n")
    
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
    
    // 2. HTTP GET using mg.get() - Simple API
    console.log("--- Test 1: Simple GET request ---")
    try {
        let body = await mg.get("http://httpbin.org/get")
        console.log("Response length:", body.byteLength, "bytes")
        console.log("Response text:", body.toString().substring(0, 200) + "...")
    } catch (e) {
        console.log("GET request failed:", e)
    }
    
    console.log("")
    
    // 3. HTTP GET using mg.request() - Advanced API with full control
    console.log("--- Test 2: Advanced GET with request() ---")
    try {
        let responseBody = null
        await mg.request("http://httpbin.org/get", (conn, event, data, timeTick) => {
            console.log("Event:", event)
            
            if (event === "connect") {
                console.log("Connected, sending request...")
                conn.send("GET /get HTTP/1.0\r\n")
                conn.send("Host: httpbin.org\r\n")
                conn.send("User-Agent: BeShell-MG\r\n")
                conn.send("\r\n")
            }
            else if (event === "http.msg") {
                console.log("Response received!")
                responseBody = data.body()
            }
        })
        
        if (responseBody) {
            console.log("Response:", responseBody.toString().substring(0, 200) + "...")
        }
    } catch (e) {
        console.log("Request failed:", e)
    }
    
    console.log("")
    
    // 4. HTTP POST request
    console.log("--- Test 3: POST request ---")
    try {
        let postData = JSON.stringify({
            device: "ESP32",
            message: "Hello from BeShell-MG"
        })
        
        let body = await mg.post("http://httpbin.org/post", postData.toArrayBuffer())
        console.log("POST Response:", body.toString().substring(0, 300) + "...")
    } catch (e) {
        console.log("POST request failed:", e)
    }
    
    console.log("")
    
    // 5. Parse URL
    console.log("--- Test 4: Parse URL ---")
    let urlInfo = mg.parseUrl("http://example.com:8080/path/to/resource")
    console.log("Parsed URL:", JSON.stringify(urlInfo))
    
    console.log("\n=== All tests completed ===")
}

main()
