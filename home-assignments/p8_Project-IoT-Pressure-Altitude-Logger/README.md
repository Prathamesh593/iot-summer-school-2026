# Sea-Level Pressure & Altitude — How It Works

## The physics

Atmospheric pressure drops as you go up. The BMP280 measures **absolute pressure** (in hPa) at wherever it physically is — it has no idea what altitude that corresponds to unless you tell it what pressure to expect at sea level.

The barometric formula used to convert pressure → altitude is:

```
altitude = 44330 * (1 - (P / P0)^0.1903)
```

Where:
- `P` = the pressure the sensor is reading right now (hPa)
- `P0` = the sea-level reference pressure (hPa) — this is the value you must supply
- `44330` and `0.1903` are constants derived from the standard atmosphere model

This is exactly what `bmp.readAltitude(seaLevelPressure)` computes internally in the Adafruit library. `seaLevelPressure` is your `P0`.

## Why P0 matters so much

If `P0` is wrong, every altitude reading is wrong by a predictable but incorrect offset — because the formula has no other way to know where "0 meters" is. A weather station 500 km away might report a `P0` of 1013 hPa while your actual local sea-level-equivalent pressure is 1008 hPa; use the wrong one and your computed altitude will be off by tens of meters, even though the sensor itself is working perfectly.

This is also why real weather reports quote pressure "adjusted to sea level," not the raw station reading — a mountain weather station reads a much lower raw pressure than a coastal one, purely due to elevation, not weather.

## Calibrating P0 (the one-time step)

If you know your actual local altitude (from GPS, a map, or a known landmark), you can back-calculate `P0` from a single live pressure reading:

```
P0 = P_measured / (1 - altitude_known / 44330)^5.255
```

This is the inverse of the altitude formula above. You do this **once**, at a known location, and reuse that `P0` for all subsequent readings — that's what `calibrateAltitude()` does in the sketch.

## The bug this project ran into

The original code recalculated `seaLevelPressure` from the known altitude **on every single loop iteration**, then immediately fed that same `seaLevelPressure` back into `bmp.readAltitude()`. Substituting the calibration formula into the altitude formula makes the `P` term cancel out algebraically — the result is just `altitude ≈ known_altitude`, regardless of what the sensor actually measured. The live pressure reading was effectively discarded.

**Fix:** calibrate `P0` once (at setup, or on a manual trigger), then use that fixed value for every reading afterward. Only the *pressure* should change loop to loop — `P0` should not.

## Quick sanity checks

- Sea-level-equivalent pressure is almost always **950–1050 hPa** anywhere on Earth's surface, regardless of your actual elevation. If your sensor is producing raw values wildly outside that (tens or hundreds of hPa), that's a wiring/power problem, not a calibration problem.
- If altitude tracks a dial/potentiometer instead of changing when you physically move the sensor, `P0` is being recomputed every loop — the bug above.
- A genuine BMP280 reports **Sensor ID `0x58`** — check this once at boot to rule out a wiring or counterfeit-chip issue before chasing calibration math.
