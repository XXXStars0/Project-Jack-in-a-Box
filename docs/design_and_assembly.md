# 🎨 Design & Assembly

## ⚙️ Hardware Components

- Raspberry Pi Pico W
- RGB LED (Common Cathode)
- Potentiometer (Model: **3386W-1-103**, 10kΩ, with top-knob) 
- MG 996R Servo (GP16 control, VBUS power)
- 100µF Capacitor (between VBUS and GND)
- 3× 220Ω Resistors (for RGB LED current limiting)
- Half-size Breadboard (Note: Full-size will not fit in the enclosure)
- Jumper wires

## 🎨 Design

### Prototypes & Schematic
![Breadboard Assembly](../img/breadboard_Assembly.jpeg)
*Current breadboard assembly for component testing and logic verification.*

![Wokwi Design](../img/Wokwi_Design.png)
*Wokwi circuit simulation for digital twin development.*

### Laser Cutting Configuration
The enclosure design files are published in **SVG** format. The following settings are calibrated for the **Epilog Fusion Pro 36**:

| Color (RGB)            | Mode         | Settings (S/P/F) | Recommended Use                |
| :--------------------- | :----------- | :--------------- | :----------------------------- |
| **Grey** (128,128,128) | Engrave      | 80 / 100 / --    | Decorative patterns / Text     |
| **Blue** (0,0,255)     | Vector Score | 50 / 25 / 10     | Fold lines / Layout guidelines |
| **Green** (0,255,0)    | Vector Cut   | 25 / 100 / 10    | Outer profile / Hole cutting   |
| **Red** (255,0,0)      | Vector Cut   | 25 / 100 / 10    | Final outer perimeter          |
| **Cyan** (0,255,255)   | Ignore       | --               | Labeling / Annotations         |
| **Black** (0,0,0)      | Ignore / Cut | --               | 30cm x 30cm Bounding Frame     |

**Recommended Operation Sequence:**
1. **Engrave** (Grey) → 2. **Vector Score** (Blue) → 3. **Vector Cut** (Green) → 4. **Vector Cut** (Red)

> [!NOTE]
> The project is currently under active development. The **Laser Cutting** enclosure design files and final assembly documentation are currently being finalized.

### 🛠️ Manufacturing & Assembly

#### Design Files
- [Jack_Design_Lazercut.svg](../design/Jack_Design_Lazercut.svg): Full laser cutting layout including enclosure and internal mechanisms.
- [Jack_Addon_1.svg](../design/Jack_Addon_1.svg): Reinforcement frame. If finger joints are loose, use one or two of these external frames to wrap and secure the box without glue.

#### Additional Hardware (Non-wood parts)
| Item                        | Specification              | Quantity |
| :-------------------------- | :------------------------- | :------- |
| **Phillips Pan Head Screw** | #4-40 x 3/8", Carbon Steel | 15 + 2   |
| **Hex Nut**                 | #4-40, Carbon Steel        | 15 + 2   |
| **Rubber Band (Optional)**  | Standard                   | 1        |

> [!TIP]
> **Customization**: The 2 extra sets of screws/nuts are provided so you can mount custom stickers or notes onto the pop-up "Jack" card.

#### Assembly Tips & Notes
- **Side Parts**: The parts labeled "Side" are interchangeable. Their orientation depends on your preferred wiring path and which side you want the handle (potentiometer) to be on.
- **Potentiometer Stability**: Due to varying laser kerf, the square cutout for the potentiometer might not be perfectly snug. If it feels loose, wrap a small piece of paper or tape around the component before mounting.
- **Tensioning**: Using a rubber band to connect the signature card and the internal partition can improve the "pop" effect. However, be careful not to overtension, as it may cause servo displacement. Consider using an additional strap or rubber band anchored to the partition for better stability.
- **Mounting**: While the design aims for a glue-less assembly, adhesive-backing the breadboard to the bottom of the box is highly recommended for the best experience.
- **Structural Integrity**: The box uses finger joints. If the fit is unstable due to laser kerf, you can use the provided **Jack_Addon_1.svg** frames to wrap the box externally. The prototype shown in the images uses two such frames for maximum stability. For a more "finished" look, wood glue or internal tape may be used instead.

> [!NOTE]
> - **Assembly Manual**: Currently under development.
> - **Mechanical Optimization**: Improved limit screw designs (Medium priority) and screw-based board mounting (Low priority) are currently being finalized.

## 💡 Pin Mapping Reference

| Component           | Pico W Pin                   | Note         |
| ------------------- | ---------------------------- | ------------ |
| **RGB LED**         | GP13 (R), GP14 (G), GP15 (B) | PWM Support  |
| **Potentiometer**   | GP27                         | Analog Input |
| **Servo (MG 996R)** | GP16                         | PWM Control  |
