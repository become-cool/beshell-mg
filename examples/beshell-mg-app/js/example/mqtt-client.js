import * as mg from "mg"
import * as wifi from "wifi"

const ssid = "your SSID"
const pwd = "your PASSWORD"

// MQTT Broker configuration
// You can use public test brokers like:
// - mqtt://broker.emqx.io:1883 (EMQX Public Broker)
// - mqtt://test.mosquitto.org:1883 (Mosquitto Test Broker)
const MQTT_BROKER = "mqtt://broker.emqx.io:1883"
const MQTT_TOPIC = "beshell-mg/demo"

async function main() {
    console.log("\n=== MQTT Client Example ===\n")
    
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
    
    // 2. Connect to MQTT Broker
    console.log("Connecting to MQTT broker:", MQTT_BROKER)
    
    let client = null
    
    try {
        client = mg.connect(MQTT_BROKER, (event, data) => {
            console.log("MQTT Event:", event)
            
            if (event === "mqtt.open") {
                console.log("Connected to MQTT broker!")
                
                // Subscribe to topic
                console.log("Subscribing to topic:", MQTT_TOPIC)
                client.sub(MQTT_TOPIC)
                
                // Publish a message
                let message = "Hello from BeShell-MG! Time: " + Date.now()
                console.log("Publishing message:", message)
                client.push(MQTT_TOPIC, message.toArrayBuffer())
                
                // Publish JSON data
                let jsonData = JSON.stringify({
                    device: "ESP32",
                    platform: "BeShell-MG",
                    timestamp: Date.now(),
                    data: {
                        temperature: 25.5,
                        humidity: 60
                    }
                })
                client.push(MQTT_TOPIC + "/json", jsonData.toArrayBuffer())
            }
            else if (event === "mqtt.msg") {
                // Received message
                let topic = data.topic()
                let payload = data.body()
                console.log("Received message on topic:", topic)
                console.log("Payload:", payload.toString())
            }
            else if (event === "mqtt.cmd") {
                console.log("MQTT command received")
            }
            else if (event === "error") {
                console.log("MQTT Error:", data)
            }
            else if (event === "close") {
                console.log("MQTT connection closed")
            }
        })
        
        // Keep the script running
        console.log("\nMQTT client is running...")
        console.log("Press Ctrl+C to stop\n")
        
        // Auto disconnect after 30 seconds for demo
        setTimeout(() => {
            console.log("\nAuto disconnecting after 30 seconds...")
            if (client) {
                client.disconnect()
            }
        }, 30000)
        
    } catch (e) {
        console.log("Failed to connect to MQTT broker:", e)
    }
}

main()
