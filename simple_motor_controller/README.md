# Simple Motor Controller (Pico)

This project exposes a minimal motor control API over I2C. It is intended to be driven by a Raspberry Pi Zero 2W (or any I2C master) that provides HTTPS, camera, and user interface features.

## Build

```bash
export PICO_SDK_PATH=/path/to/pico-sdk
mkdir -p build
cd build
cmake -DPICO_BOARD=pico2_w ..
make
```

## I2C Wiring (Pi Zero 2W <-> Pico)

- Pi Zero 2W SDA: GPIO2 (pin 3)
- Pi Zero 2W SCL: GPIO3 (pin 5)
- Pico SDA: GP2 (pin 4)
- Pico SCL: GP3 (pin 5)
- Common GND: connect Pi GND to Pico GND

The Kitronik motor board uses the Pico I2C0 bus on GP8/GP9 by default.

## Protocol

The Pico runs as an I2C slave at address `0x12`. The master writes a JSON payload and reads a status JSON string.

### Write (master -> Pico)

Example payloads:

```json
{"w":1,"x":0,"y":0,"z":0}
```

```json
{"stop":true}
```

### Read (master <- Pico)

Example response:

```json
{"status":"ok"}
```

## Notes

- JSON parsing is minimal and expects well-formed input.
- `stop:true` immediately stops all motors.
- Status responses include: `ready`, `ok`, `stopped`.
