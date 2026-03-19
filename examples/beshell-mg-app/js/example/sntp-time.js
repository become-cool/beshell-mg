import * as mg from "mg"
import * as wifi from "wifi"

const ssid = "your SSID"
const pwd = "your PASSWORD"

// SNTP servers
const SNTP_SERVERS = [
    "udp://pool.ntp.org:123",
    "udp://time.windows.com:123",
    "udp://time.google.com:123"
]

async function main() {
    console.log("\n=== SNTP Time Synchronization Example ===\n")
    
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
    
    // 2. Configure DNS
    console.log("Current DNS server:", mg.getDNS())
    console.log("Current DNS timeout:", mg.getDNSTimeout(), "ms")
    console.log("")
    
    // 3. Query SNTP servers
    console.log("Querying SNTP servers...")
    console.log("")
    
    for (let server of SNTP_SERVERS) {
        console.log("Trying server:", server)
        
        try {
            let timestamp = await new Promise((resolve, reject) => {
                mg.sntpRequest(server, (err, time) => {
                    if (err) {
                        reject(err)
                    } else {
                        resolve(time)
                    }
                })
            })
            
            let date = new Date(timestamp)
            console.log("  Success! Time:", date.toISOString())
            console.log("  Timestamp (ms):", timestamp)
            console.log("  Local time:", date.toLocaleString())
            
        } catch (e) {
            console.log("  Failed:", e)
        }
        
        console.log("")
    }
    
    // 4. DNS Cache operations demo
    console.log("=== DNS Cache Demo ===")
    
    // Add a DNS cache entry
    console.log("Adding DNS cache entry: test.local -> 192.168.1.100")
    mg.addDNSCache("test.local", "192.168.1.100", 60000)
    
    // Remove a DNS cache entry
    console.log("Removing DNS cache entry: test.local")
    mg.removeDNSCache("test.local")
    
    // Clear all DNS cache
    console.log("Clearing all DNS cache...")
    mg.clearDNSCache()
    
    console.log("\n=== SNTP demo completed ===")
}

main()
