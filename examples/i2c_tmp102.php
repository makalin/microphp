<?php
// I2C TMP102 Temperature Sensor example
// This demonstrates I2C communication and sensor data processing

// Open I2C bus 0 on pins SCL=22, SDA=21 with 400kHz speed
$i2c = i2c_open(0, scl: 22, sda: 21, speed: 400000);

if ($i2c === false) {
    echo "Failed to open I2C bus\n";
    exit(1);
}

echo "I2C TMP102 temperature sensor example\n";
echo "Reading temperature every 2 seconds...\n\n";

// TMP102 register addresses
$TEMP_REG = 0x00;  // Temperature register
$TMP102_ADDR = 0x48;  // TMP102 I2C address

while (true) {
    // Write to pointer register to select temperature register
    if (i2c_write($i2c, $TMP102_ADDR, [$TEMP_REG]) !== true) {
        echo "Failed to write to TMP102\n";
        sleep_ms(2000);
        continue;
    }
    
    // Read 2 bytes of temperature data
    $raw_data = i2c_read($i2c, $TMP102_ADDR, 2);
    
    if ($raw_data === false) {
        echo "Failed to read from TMP102\n";
        sleep_ms(2000);
        continue;
    }
    
    // Convert raw data to temperature
    // TMP102: 12-bit resolution, 0.0625°C per LSB
    $temp_raw = ($raw_data[0] << 4) | ($raw_data[1] >> 4);
    
    // Convert to Celsius (signed 12-bit)
    if ($temp_raw & 0x800) {
        $temp_raw = $temp_raw - 0x1000;  // Sign extend negative values
    }
    
    $temp_celsius = $temp_raw * 0.0625;
    
    // Convert to Fahrenheit
    $temp_fahrenheit = ($temp_celsius * 9/5) + 32;
    
    // Get current timestamp
    $timestamp = millis();
    
    // Display results
    printf("Time: %d ms, Temperature: %.2f°C (%.2f°F)\n", 
           $timestamp, $temp_celsius, $temp_fahrenheit);
    
    // Wait 2 seconds before next reading
    sleep_ms(2000);
}

// Note: This loop runs indefinitely
// In a real application, you might want to add a way to exit
?>
