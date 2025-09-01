<?php
// Blink example - Blinks an LED connected to GPIO pin 2
// This demonstrates basic GPIO control and timing

// Set GPIO pin 2 as output
gpio_mode(2, OUTPUT);

// Main blink loop
while (true) {
    // Turn LED on
    gpio_write(2, true);
    
    // Wait 200ms
    sleep_ms(200);
    
    // Turn LED off
    gpio_write(2, false);
    
    // Wait 800ms
    sleep_ms(800);
}
?>
