# Wiring Sketch

Simple power layout for Pico + Kitronik motor board + Pi Zero 2W on a shared 5V supply.

```
Dual Li-ion Pack -> DFRobot 5V Regulator (1.5A continuous / 3A peak)
                         |
                         +---> 5V ----> Pi Zero 2W 5V (pin 2/4 or USB)
                         |              |
                         |              +---> 470–1000 uF cap ---> GND (near Pi)
                         |
                         +---> 5V ----> Pico VSYS
                         |
                         +---> 5V ----> Kitronik Motor Board 5V input

DFRobot GND ------------------------------------------+--------------------+--------------------+
                                                      |                    |                    |
                                                      +--> Pi Zero 2W GND   +--> Pico GND         +--> Kitronik GND

All GNDs tied together (star back to regulator if possible).

I2C wiring (Pi Zero 2W <-> Pico):
  - Pi Zero 2W SDA: GPIO2 (pin 3)
  - Pi Zero 2W SCL: GPIO3 (pin 5)
  - Pico SDA: GP4 (pin 6) or GP6 (pin 9)
  - Pico SCL: GP5 (pin 7) or GP7 (pin 10)
  - Common GND: connect Pi GND to Pico GND
```

Notes:
- Do not feed Pico 3.3V and VSYS at the same time.
- Keep motor power wiring short; avoid routing Pi power through the motor board.
