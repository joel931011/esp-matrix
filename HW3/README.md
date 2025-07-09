# ESP32 LED Bouncing Ball Game

## Description
This project runs on ESP32 and controls an 8×8 LED matrix (via MAX7219) to display a bouncing ball constrained by a wall on the left.

## Configuration
- Frame rate: 5 fps
- Initial position: (1, 1)
- End position: (6, 4)

## Files
- `main.c`: Core source file
- `CMakeLists.txt`: Build configuration
- `README.md`: This description

## How to Build
```bash
idf.py build
idf.py -p [PORT] flash monitor
