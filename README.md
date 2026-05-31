# Ultimate Realistic Open World Motorcycle Simulator

Phase 1: Core Prototype — basic motorcycle movement on a simple map.

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./UltimateMotorcycle
```

## Controls

| Key              | Action       |
|------------------|-------------|
| W / Up Arrow     | Accelerate  |
| S / Down Arrow   | Brake       |
| A / Left Arrow   | Steer Left  |
| D / Right Arrow  | Steer Right |
| R                | Reset Bike  |
| ESC              | Quit        |

## Phases

- Phase 1: Core prototype (this)
- Phase 2: Motorcycle realism (gears, clutch, crashes)
- Phase 3: Environment build (city, traffic, weather)
- Phase 4: Open world expansion
- Phase 5: Multiplayer system
- Phase 6: Polish & realism enhancement
