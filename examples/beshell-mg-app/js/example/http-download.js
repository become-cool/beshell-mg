import * as mg from "mg"
import * as wifi from "wifi"
import * as fs from "fs"

const ssid = "your SSID"
const pwd = "your PASSWORD"

// URL to download (small file for testing)
const DOWNLOAD_URL = "http://httpbin.org/bytes/1024"
const SAVE_PATH = "/data/downloaded.bin"

async function main() {
    console.log("\n=== HTTP Download Example ===\n")
    
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
    
    // 2. Ensure /data directory exists
    try {
        fs.mkdir("/data")
    } catch (e) {
        // Directory might already exist
    }
    
    // 3. Download to memory
    console.log("--- Test 1: Download to memory ---")
    try {
        let data = null
        await mg.download(DOWNLOAD_URL, null, (total, current, chunk) => {
            console.log(`Progress: ${current}/${total} bytes (${(current/total*100).toFixed(1)}%)`)
            data = chunk
        })
        console.log("Downloaded to memory, size:", data ? data.byteLength : 0, "bytes")
    } catch (e) {
        console.log("Download failed:", e)
    }
    
    console.log("")
    
    // 4. Download to file
    console.log("--- Test 2: Download to file ---")
    console.log("Downloading to:", SAVE_PATH)
    
    try {
        await mg.download(DOWNLOAD_URL, SAVE_PATH, (total, current, chunk) => {
            console.log(`Progress: ${current}/${total} bytes (${(current/total*100).toFixed(1)}%)`)
        })
        console.log("Download completed!")
        
        // Verify file
        let stat = fs.stat(SAVE_PATH)
        console.log("File size:", stat.size, "bytes")
        
        // Read and display first 100 bytes
        let fhandle = fs.open(SAVE_PATH, "r")
        let buffer = new Uint8Array(100)
        fs.read(fhandle, buffer)
        fs.close(fhandle)
        
        console.log("First 100 bytes (hex):", Array.from(buffer).map(b => b.toString(16).padStart(2, '0')).join(' '))
        
        // Clean up
        console.log("Removing downloaded file...")
        fs.unlink(SAVE_PATH)
        
    } catch (e) {
        console.log("Download failed:", e)
    }
    
    console.log("\n=== Download demo completed ===")
}

main()
