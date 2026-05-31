# Ultimate Realistic Open World Motorcycle Simulator

Phase 2: Motorcycle Realism System — realistic physics, gears, clutch, crashes.

## Controls

| Key               | Action         |
|------------------|----------------|
| W / Up Arrow     | Throttle       |
| S / Down Arrow   | Rear Brake     |
| Space            | Front Brake    |
| Left Shift       | Clutch         |
| A / Left Arrow   | Steer Left     |
| D / Right Arrow  | Steer Right    |
| Q                | Gear Up        |
| E                | Gear Down      |
| Enter / Ctrl     | Start Engine   |
| R                | Reset Bike     |
| ESC              | Quit           |

## Gamepad

| Input              | Action         |
|-------------------|----------------|
| Right Trigger     | Throttle       |
| Left Trigger      | Clutch         |
| Left Stick X      | Steer          |
| LB                | Rear Brake     |
| RB                | Front Brake    |
| A / Y             | Start Engine   |
| B                 | Reset Bike     |
| D-Pad Up / Y      | Gear Up        |
| D-Pad Down / X    | Gear Down      |

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./UltimateMotorcycle
```

## Phase 2 Systems

- **Engine**: RPM, stall, rev limiter, temperature, fuel consumption
- **Transmission**: 5-speed gearbox + neutral, manual clutch, half-clutch, gear clash
- **Chassis**: Center of gravity, lean physics, low-speed wobble, high-speed stability
- **Suspension**: Front/rear spring-damper, weight transfer, pothole response
- **Tires**: Grip per surface, slip angle, lock-up, wheel spin
- **Rider**: Ragdoll physics, 6-joint body, crash ejection, slide/flip physics
- **Surface System**: Asphalt, wet road, gravel, dirt, mud, sand, grass, potholes

## Phases

- Phase 1: Core prototype (done)
- Phase 2: Motorcycle realism (current)
- Phase 3: Environment build (city, traffic, weather)
- Phase 4: Open world expansion
- Phase 5: Multiplayer system
- Phase 6: Polish & realism enhancement
