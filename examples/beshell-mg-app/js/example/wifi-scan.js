import * as wifi from "wifi"

async function main(){
    console.log("\n=== WiFi Scan Example ===\n")
    
    console.log("Scanning for WiFi networks...")
    console.log("")
    
    let networks = await wifi.scan()
    
    console.log("Found", networks.length, "networks:")
    console.log("")
    console.log("SSID                          RSSI    Channel  Auth Mode")
    console.log("------------------------------------------------------------")
    
    // Sort by RSSI (signal strength)
    networks.sort((a, b) => b.rssi - a.rssi)
    
    for (let net of networks) {
        let ssid = net.ssid.padEnd(30)
        let rssi = (net.rssi + " dBm").padEnd(8)
        let channel = (net.channel + "").padEnd(9)
        let auth = net.authmode || "Unknown"
        
        console.log(ssid, rssi, channel, auth)
    }
    
    console.log("")
    console.log("Scan completed!")
}

main()
