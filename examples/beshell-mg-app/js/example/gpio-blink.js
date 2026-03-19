import * as gpio from "gpio"

const LED_PIN = 2  // Change this to your board's LED pin

async function main() {
    console.log("\n=== GPIO Blink Example ===\n")
    console.log("Blinking LED on pin", LED_PIN)
    console.log("Press Ctrl+C to stop\n")
    
    // Configure pin as output
    gpio.pinMode(LED_PIN, gpio.OUTPUT)
    
    let state = 0
    
    while (true) {
        gpio.digitalWrite(LED_PIN, state)
        state = state ? 0 : 1
        await new Promise(r => setTimeout(r, 500))
    }
}

main()
