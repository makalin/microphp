# micro-PHP

> **Tiny PHP runtime (≈1–2 MB)** baked straight into firmware.
> Targets **ESP32** (ESP-IDF) and **Raspberry Pi Pico / RP2040** (Pico SDK) for **IoT scripting** without Linux.

[![Repo](https://img.shields.io/badge/GitHub-makalin%2Fmicrophp-black?logo=github)](https://github.com/makalin/microphp)
![Size](https://img.shields.io/badge/runtime-~1.6MB-blue)
![Boards](https://img.shields.io/badge/boards-ESP32%20%7C%20RP2040-green)
![License](https://img.shields.io/badge/license-MIT-lightgrey)

---

## Why

* Familiar **PHP syntax** for quick device scripts.
* **Static firmware**: no shell, no dynamic loading, optional VFS only.
* Deterministic footprint: **≤2 MB** VM, **≤256 KB** bytecode heap.

---

## Architecture

* **Zend-lite VM**: compact opcode interpreter.
* **AOT compiler** (`microphpc`): PHP → **MBC bytecode** → embedded as a `const uint8_t[]`.
* **Built-ins** for MCU I/O: GPIO, PWM, ADC, I2C, SPI, UART, RTC, and Wi-Fi/TCP on ESP32.
* **Memory**: arenas for zvals/arrays/strings, RC + COW, interned ids, optional exceptions.
* **No** `eval`, dynamic `include/require`, PCRE, iconv/mbstring, PDO/curl/streams zoo.

```
your.php  --microphpc-->  your.mbc  --objgen-->  embedded_program.c  → firmware.bin
```

---

## Project status

* VM/core: ✅ arrays, strings, closures, errors
* HAL: ✅ GPIO/PWM/UART | ⚠️ I2C/SPI alpha | 🌐 Wi-Fi (ESP32 alpha)
* Tooling: ✅ `microphpc`, `mbc-inspect`, `objgen.py`
* Boards: ✅ ESP32-S3 | ✅ RP2040 | ☑️ ESP32-C3 (CI pending)

---

## Quick start

### Toolchains

* **ESP32**: ESP-IDF v5.x (`idf.py`, `xtensa-esp32-elf` or `riscv32-esp-elf`)
* **RP2040**: Pico SDK + `arm-none-eabi-gcc`
* Host: CMake ≥3.20, Python ≥3.9, PHP ≥8.1 (lint only)

### Build tools

```bash
git clone https://github.com/makalin/microphp.git
cd microphp/tools/microphpc
cmake -B build -S .
cmake --build build --config Release
```

### Compile PHP → MBC

```bash
echo '<?php gpio_write(2,true); sleep_ms(500); gpio_write(2,false); ?>' > examples/blink.php
./build/microphpc examples/blink.php -o out/blink.mbc
```

### Embed & flash (ESP32 example)

```bash
cd ../../targets/esp32
python tools/objgen.py ../../out/blink.mbc > main/embedded_program.c
idf.py set-target esp32s3
idf.py build flash monitor
```

### RP2040 (Pico SDK)

```bash
cd ../rp2040
cmake -B build -S .
cmake --build build --config Release
picotool load -x build/microphp.uf2
```

---

## Examples

### Blink

```php
<?php
gpio_mode(2, OUTPUT);
while (true) {
  gpio_write(2, true);  sleep_ms(200);
  gpio_write(2, false); sleep_ms(800);
}
```

### PWM fade

```php
<?php
$p = pwm_open(0, pin: 15, freq_hz: 5000, duty: 0.0);
for ($d=0.0; $d<=1.0; $d+=0.02) { pwm_set($p,$d); sleep_ms(20); }
for ($d=1.0; $d>=0.0; $d-=0.02) { pwm_set($p,$d); sleep_ms(20); }
```

### I2C (TMP102)

```php
<?php
$i2c = i2c_open(0, scl:22, sda:21, speed:400000);
i2c_write($i2c, 0x48, [0x00]);
$raw = i2c_read($i2c, 0x48, 2);
$temp = (($raw[0] << 4) | ($raw[1] >> 4)) * 0.0625;
printf("T=%.2f C\n", $temp);
```

### Minimal HTTP (ESP32)

```php
<?php
wifi_connect(ssid:"SSID", pass:"PASS");
$s = http_listen(80);
while ($c = http_accept($s, 5000)) {
  $req = http_read($c);
  http_write($c, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello micro-PHP");
  http_close($c);
}
```

---

## Built-ins (HAL)

* **Time**: `sleep_ms(int)`, `millis(): int`
* **GPIO**: `gpio_mode(pin,int)`, `gpio_write(pin,bool)`, `gpio_read(pin): bool`
* **PWM**: `pwm_open(ch,pin,freq_hz,duty)`, `pwm_set(h,duty)`, `pwm_close(h)`
* **UART**: `uart_open(id,baud)`, `uart_read(h,n)`, `uart_write(h,bytes)`
* **I2C/SPI**: `i2c_open/read/write`, `spi_txrx`
* **Net/ESP32**: `wifi_connect`, `tcp_*`, `udp_*`, `http_*`, `net_time_sync`
* **RTC/Flash**: `rtc_now()`, `kv_get/kv_set` (tiny flash KV)

---

## Build configuration (CMake)

| Option                | Default | Notes                         |
| --------------------- | ------- | ----------------------------- |
| `MICROPHP_EXCEPTIONS` | ON      | Disable to save \~8–20 KB     |
| `MICROPHP_FLOAT64`    | ON      | OFF → float32, faster/smaller |
| `MICROPHP_STDIO`      | ON      | UART logging                  |
| `MICROPHP_NET`        | OFF     | ESP32 networking              |
| `MICROPHP_KVSTORE`    | ON      | Flash KV store                |

Memory knobs: `MICROPHP_STR_ARENA_KB` (128), `MICROPHP_ARRAY_ARENA_KB` (128), `MICROPHP_STACK_KB` (24), `MICROPHP_TASKS_MAX` (4).

---

## Benchmarks\* (ESP32-S3 @ 240 MHz)

* Array push 1e5: **\~45 ms**
* String concat 1e5 (8 B): **\~64 ms**
* GPIO tight toggle: **\~1.1 MHz** effective

\*Synthetic; varies by config/board.

---

## FAQ

**Can I run WordPress?** No—this is an embedded subset. Think **small, single-purpose** scripts.
**Filesystem?** Optional RAM/flash VFS; typical flow embeds bytecode.
**“Hello, world” size?** \~1.6 MB runtime (ESP32, no NET) + \~0.5–2 KB bytecode.

---

## Contributing

* Keep features behind toggles; every byte counts.
* Include **size diff** and `.map` excerpts in PRs.
* Add **example** + `mbc-inspect` output for new built-ins.

---

## Roadmap

* Sandboxed VFS (ramfs/kvfs)
* TLS via mbedTLS (ESP32)
* `.mphar` bundler (single blob)
* Super-op fusion in `microphpc -O3`
* On-target trace profiler & flamegraph dump

---

## License

MIT (runtime & tools). Vendor HALs retain their licenses.

---

## Repo layout

```
/core        # VM, zval, arrays, strings, opcodes
/hal         # esp32, rp2040 ports
/tools       # microphpc, mbc-inspect, objgen
/targets     # firmware projects (ESP-IDF, Pico SDK)
/examples    # blink, pwm, i2c, http, kv, uart
/docs        # internals, opcodes, ABI
```

---

## Credits & Acknowledgements

* **Core design & implementation**: [@makalin](https://github.com/makalin)
* **PHP inspiration**: [PHP Group](https://www.php.net/) and the **Zend Engine** architecture
* **MCU SDKs**:

  * [ESP-IDF](https://github.com/espressif/esp-idf) (ESP32)
  * [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk) (RP2040)
* **Toolchain support**:

  * `xtensa-esp32-elf` and `riscv32-esp-elf` GCC ports
  * `arm-none-eabi-gcc` for ARM Cortex-M0+
* **Community inspiration**: lightweight interpreters like [MicroPython](https://micropython.org/) and [Lua RTOS](https://github.com/whitecatboard/Lua-RTOS-ESP32)
* **Contributors**: see [GitHub contributors](https://github.com/makalin/microphp/graphs/contributors)

---

> 🐾 **micro-PHP — Scripting the tiny world, one byte at a time.**
