import * as wifi from "wifi"

const AP_SSID = "BeShell-MG-AP"
const AP_PWD = "12345678"

async function main(){
    console.log("\n=== WiFi Access Point Example ===\n")
    
    console.log("Starting AP mode...")
    console.log("SSID:", AP_SSID)
    console.log("Password:", AP_PWD)
    
    // Start AP mode
    wifi.apStart({
        ssid: AP_SSID,
        password: AP_PWD,
        channel: 6,
        max_connection: 4
    })
    
    console.log("\nAP started!")
    console.log("Connect to '" + AP_SSID + "' and open http://192.168.4.1 in your browser")
    console.log("\nPress Ctrl+C to stop\n")
    
    // Keep running
    while(true) {
        let stations = wifi.apGetStations()
        if (stations.length > 0) {
            console.log("Connected stations:", stations.length)
            for (let sta of stations) {
                console.log("  -", sta.mac, "IP:", sta.ip)
            }
        }
        await new Promise(r => setTimeout(r, 5000))
    }
}

main()
