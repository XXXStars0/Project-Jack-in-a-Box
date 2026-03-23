# 🛠️ Troubleshooting & Fixes

### Eduroam Connection Failure
- **Issue:** Attempted to support `eduroam` (WPA2 Enterprise) directly, but the Pico W's `WiFi.h` library (arduino-pico) does not natively support the EAP authentication required for university networks.
- **Fix:** Switched to the **RedRover** IoT registration method. By registering the device's MAC address with Cornell IT, the Pico W can connect to the internet without additional enterprise headers. WiFi connection is now stable.
- **Note:** Ensure `Jack/keys.h` is correctly updated with `SECRET_SSID "RedRover"` and an empty `SECRET_PASS ""` to avoid authentication errors.

### Wired Mode Deprecated
- **Note:** The USB/Processing wired mode has been removed from the firmware. The project now operates exclusively via Wi-Fi. Legacy Processing files remain in the repository for reference only.

### Design File Scaling Issues (Fusion 360 to Illustrator)
- **Issue:** DXF files exported from Fusion 360 often import into Adobe Illustrator with broken scales, leading to incorrect physical dimensions during laser cutting.
- **Fix:** When importing DXF into Illustrator, ensure the scaling options are set explicitly (e.g., **1 Unit = 1 mm**). Using Adobe Illustrator for the final SVG export allows for superior color-coding (RGB mapping) and labeling required for laser cutter software.

### Mechanical Friction & Reset Issues
- **Issue:** The mechanism uses a cam to lift a wooden lever (the "Jack" card) which then pushes the lid open. High friction in the wooden laser-cut pivots can prevent the cam from returning reliably to the closed position, making the reset less smooth than the original LEGO prototype.
- **Workarounds:**
  - **Tensioning:** Attach a rubber band between the Jack card and the internal partition to force a return. 
  - **Servo Stability:** High tension from rubber bands may shift the servo; use an additional strap or rubber band to anchor the servo body more firmly to the partition.
  - **Polishing:** Lightly sand or polish the wooden axles and pivoting points to reduce surface friction.
  - **Future Fix:** A direct screw-mount for the servo is under consideration but remains low priority due to manufacturing complexity.

### Potentiometer Fit & Tolerances
- **Issue:** The square cutout for the potentiometer body and the circular hole for the knob rely on tight laser-cutting tolerances. Real-world kerf can result in a loose or overly tight fit compared to the theoretical CAD model.
- **Fixes:**
  - **Looseness:** If the square cutout is too large, wrap a thin strip of paper or tape around the potentiometer body to create a snug friction fit.
  - **Tightness:** If the hole is too small, it is recommended to re-cut the part with adjusted kerf compensation.
- **Current Prototype Status:** The example box shown in this project features a slightly loose square cutout (fixed with thing paper strips) and a perfectly fitting circular knob hole.

### Structural Stability & Joint Fit
- **Issue:** The enclosure relies on finger joints. Depending on laser precision, these joints may be too loose to hold the box together securely without glue.
- **Fix:** Use the external reinforcement frames provided in **Jack_Addon_1.svg**. These are designed to wrap around the exterior of the box and lock the panels in place. Alternatively, a small amount of wood glue or internal double-sided tape will provide a more permanent and aesthetically clean solution.
