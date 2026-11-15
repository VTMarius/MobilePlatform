# Draft design references

These files preserve the Devastator design discussion alongside the firmware.
They are **not fabrication releases** and are not implemented by the current
ESP32 sensing application.

- `Devastator_Controller_Design_v0_1.pdf`: early component survey and concept.
  Its provisional motor-driver/interface choices were superseded later.
- `Devastator_Electrical_Design_v0_2.pdf` and `power_v0_2/`: proposed battery,
  regulator, motor-driver and power-distribution design, including a provisional
  pulse clamp. Module contacts in that schematic are logical contracts rather
  than verified physical footprint pads.
- `Devastator_StopArm_v0_1.kicad_sch` / `.pdf`: later Teensy stop/arm draft from
  the user, with separate release memory, watchdog and command-idle timer.
  The supplied schematic is preserved unchanged.
- `devastator.kicad_sym`: the custom logic symbols extracted from that schematic,
  unchanged apart from removal of their library prefix for a standalone library.
  `sym-lib-table` and the small `.kicad_pro` make the symbol library discoverable
  in KiCad 9. Footprints and exact package selections remain open.

## Latest review notes

The last connectivity review found no mismatch in 91 intended IC pin-to-net
assignments. This does not prove timing, physical assembly or protection.

1. Static READY must be verified low through every Teensy reset path before
   claiming immediate reset-to-disarm. A short-timeout heartbeat is not an
   equivalent guarantee.
2. Arming near RUN_OK recovery remains unresolved: do not assume metastable
   outcomes are harmless merely because commands are initially zero.
3. The proposed 20 ns RUN_OK-low constraint for the feedback release memory
   is not a manufacturer-characterized reset-pulse rating.
4. R12 + R13 and C5 give a 2 ms stop-input discharge time constant. Complete
   stop response also depends on Schmitt thresholds and propagation delays.
5. TPS3839K33 is nominally 2.93 V; coordinate its threshold, tolerance and delay
   with the voltage range used for timing claims. G33 is a different threshold.
6. Choose packages, footprints, TVS devices, timer-component tolerances and
   connectors before assembly/layout. Place each bypass capacitor at its IC.
7. Braking energy, clamp suitability, fuses, battery-cell protection and actual
   motor behaviour still require measurements.

Keep the ESP32 sensing prototype electrically separate from unvalidated motor
controls. The next firmware stage is mounting/axis verification and measurement,
not autonomous driving.
