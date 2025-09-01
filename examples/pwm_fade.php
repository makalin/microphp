<?php
// PWM Fade example - Fades an LED using PWM on channel 0, pin 15
// This demonstrates PWM control and floating-point math

// Open PWM channel 0 on pin 15 with 5kHz frequency and 0% duty cycle
$pwm = pwm_open(0, pin: 15, freq_hz: 5000, duty: 0.0);

if ($pwm === false) {
    echo "Failed to open PWM channel\n";
    exit(1);
}

echo "PWM fade started...\n";

// Fade up (0% to 100%)
for ($duty = 0.0; $duty <= 1.0; $duty += 0.02) {
    pwm_set($pwm, $duty);
    sleep_ms(20);  // 20ms delay for smooth fade
}

// Fade down (100% to 0%)
for ($duty = 1.0; $duty >= 0.0; $duty -= 0.02) {
    pwm_set($pwm, $duty);
    sleep_ms(20);  // 20ms delay for smooth fade
}

// Close PWM channel
pwm_close($pwm);

echo "PWM fade completed\n";
?>
