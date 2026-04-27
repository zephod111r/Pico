# Pico 2W C/C++ Port

This folder contains a Pico SDK (CMake) implementation of the same HTTP endpoints and motor control logic.

## Build

```bash
export PICO_SDK_PATH=/path/to/pico-sdk
mkdir -p build
cd build
cmake -DPICO_BOARD=pico2_w ..
make
```

## Configure Wi-Fi

Edit `cpp/include/config.h` with your SSID and password.

## BLE Provisioning

This build exposes a BLE GATT service for provisioning:

- SSID: write to characteristic `12345678-1234-5678-1234-56789abcdef1`
- Password: write to characteristic `12345678-1234-5678-1234-56789abcdef2`
- Apply: write any value to characteristic `12345678-1234-5678-1234-56789abcdef3`
- Control (write): `12345678-1234-5678-1234-56789abcdef4` (payload `w,x,y,z`)
- Status (read): `12345678-1234-5678-1234-56789abcdef5`

Credentials are persisted to flash.

## Endpoints

- `GET /` serves the control UI
- `POST /api/motors` accepts JSON `{ "w": 1, "x": 0, "y": 0, "z": 0 }`

## Wiring

See `cpp/Wiring.md` for a simple power wiring sketch.
