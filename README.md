![Header Image](doc/img/header.jpg)

# ⌚ RoutineHero – A Smartwatch Experience for Kids

> **Forked from [InfiniTime](https://github.com/InfiniTimeOrg/InfiniTime)**  
> Full credit and thanks to the original InfiniTime team for their amazing open-source smartwatch firmware project!

---

## 📘 About

**RoutineHero** is a fun and structured smartwatch experience built for kids, based on the InfiniTime firmware.  
As a parent, you define a daily routine made up of activities — like brushing teeth, reading, playtime, etc. — and the watch guides your child throughout the day.

RoutineHero displays these activities visually as **segments on a pie chart over an analog-style clock face**, giving kids a sense of time and structure in a playful and intuitive way.

---

## 🖼 Smartwatch in Action

Here are two GIFs showcasing RoutineHero on the watch:

<div align="center">
  <img src="doc/img/smartwatch_activities.gif" style="border-radius: 50px; box-shadow: rgba(0, 0, 0, 0.2) 0 4px 8px; padding: 40px; background: black; max-width: calc(100% - 80px)"><br>
  <strong>Time-lapse</strong>
  <br>
  <br>
  <img src="doc/img/smartwatch_timer.gif" style="border-radius: 50px; box-shadow: rgba(0, 0, 0, 0.2) 0 4px 8px; padding: 40px; background: black;max-width: calc(100% - 80px)"><br>
  <strong>Clock-Timer transitions</strong>
</div>

---

## 📱 App Demonstration

Short videos of the app in use:

### 🚀 Initial setup
[![Initial setup](https://img.youtube.com/vi/YKV8vJ6PzUU/maxresdefault.jpg)](https://www.youtube.com/shorts/YKV8vJ6PzUU)

App on the Play Store: https://play.google.com/store/apps/details?id=com.routinehero

---

## 🎯 Key Features

- 🧭 **Analog Clock UI with Activity Pie Chart**  
  Each segment of the analog clock represents an activity in the child’s day.

- 👨‍👩‍👧‍👦 **Parent-Defined Routine**  
  Define your child’s daily routine with a simple config (custom app or BLE tool integration planned).

- 🔔 **Gentle Time Awareness**  
  Visual reminders help kids understand when it's time to transition to the next task.

- 🎨 **Kid-Friendly Design**  
  Clean, colorful UI tailored for younger users with simplified interactions.

- 💡 **Based on InfiniTime**  
  Leverages the powerful open-source firmware platform that runs on PineTime and compatible devices.

---

## 📱 Devices Supported

RoutineHero runs on devices compatible with InfiniTime, including:

- PineTime (nRF52832)
- Dev boards supporting BLE and compatible display controllers (with porting)

## 🚀 Getting Started

To get RoutineHero running on your watch, choose one of the flashing options below. After flashing, proceed to configure your child's routine.

### Option A: Flash the pre-built DFU file over BLE (Easiest)
Use this if you already have a compatible bootloader and want to install the firmware wirelessly.

1. Download the latest DFU package from the repo:  
   [`doc/dfu/pinetime-mcuboot-app-dfu-1.15.0.zip`](doc/dfu/pinetime-mcuboot-app-dfu-1.15.0.zip)

2. Use a Bluetooth Low Energy (BLE) capable DFU tool to flash the firmware to your PineTime device, for example:
   - [Watchmate for Linux (works great for me)](https://github.com/azymohliad/watchmate)  
   - [nRF Connect (mobile app)](https://www.nordicsemi.com/Products/Development-tools/nrf-connect-for-mobile)  
   - [nRF Connect for Desktop](https://www.nordicsemi.com/Products/Development-tools/nrf-connect-for-desktop)

3. Follow the instructions in the DFU tool to upload the firmware to your watch over BLE.

---

### Option B: Flash Custom Bootloader + Pre-built Firmware via J-Link (SWD)
Use this if you want to install our custom bootloader to prevent kids from resetting the device, or if you need to flash the firmware wired.

The included `bootloader.bin` in the root of the repository is a modified version of the official PineTime bootloader. It is customized to load the application firmware immediately (disabling the button-press delay), preventing children from resetting or rolling back the firmware by holding down the button.

> [!IMPORTANT]
> Installing or updating the bootloader requires opening the watch casing and connecting an SWD debugger (like a J-Link or ST-Link). This is completely optional.

If you have a J-Link debugger connected:

1. Unzip the pre-built DFU package included in the repository to extract the firmware binary:
   ```bash
   unzip -o doc/dfu/pinetime-mcuboot-app-dfu-1.15.0.zip -d doc/dfu/
   ```

2. Run the OpenOCD command from the root of the project to flash both the custom bootloader and the extracted application firmware:
   ```bash
   openocd -f interface/jlink.cfg -c "transport select swd" -f target/nrf52.cfg -c "\
   init; halt; reset_config none; \
   flash erase_address 0x00000000 0x80000; \
   program bootloader.bin verify 0x00000000; \
   program $(ls doc/dfu/pinetime-mcuboot-app-image-1.15.*.bin) verify 0x8000; \
   reset run; exit"
   ```

---

### Option C: Build from Source and Flash (Advanced)
Use this if you want to customize the firmware code and compile it yourself.

1. **Clone the Repo**
   ```bash
   git clone https://github.com/trollderiu/InfiniTime-RoutineHero.git
   cd InfiniTime-RoutineHero
   ```

2. **Build the Firmware**  
   Follow the [InfiniTime build instructions](https://github.com/InfiniTimeOrg/InfiniTime#building-the-firmware), as they apply here too.
   ```bash
   docker pull --platform linux/amd64 infinitime/infinitime-build
   docker run --rm -it -v ${PWD}:/sources --user $(id -u):$(id -g) infinitime/infinitime-build
   ```

3. **Flash the Custom Binary**  
   - **Over BLE (DFU):** Flash the resulting DFU package at `build/output/src/pinetime-mcuboot-app-dfu-1.15.*.zip` using the tools from Option A.
   - **Over J-Link (SWD):** If you want to use J-Link with your newly compiled binary, make sure you have `bootloader.bin` in the root, and run:
     ```bash
     openocd -f interface/jlink.cfg -c "transport select swd" -f target/nrf52.cfg -c "\
     init; halt; reset_config none; \
     flash erase_address 0x00000000 0x80000; \
     program bootloader.bin verify 0x00000000; \
     program $(ls build/output/src/pinetime-mcuboot-app-image-1.15.*.bin) verify 0x8000; \
     reset run; exit"
     ```

---

### 📱 Configure Your Child’s Routine

After flashing the watch, use the official **RoutineHero Parent App** to set up daily routines and activities:

➡️ [**Download from Google Play**](https://play.google.com/store/apps/details?id=com.routinehero)

This app lets you:
- Create and manage a daily schedule
- Set icons, names, and durations for each task
- Sync routines wirelessly to the watch

*iOS support is planned for the future.*

---

## 📂 Project Structure

This project retains much of the original InfiniTime code, with key changes including:

- `src/display/RoutineHeroWatchFace.cpp` – Custom watchface with pie chart overlay
- `src/display/RoutineHeroTimer.cpp` – Screen with time left for the current activity
- `resources/images/` – Custom icons for kid-friendly interface

More detailed module descriptions coming soon.

---

## 🛠 Contributing

Want to help improve RoutineHero? PRs and suggestions are welcome!  
Check the [Issues](https://github.com/trollderiu/InfiniTime-RoutineHero/issues) tab for bugs and roadmap items.

---

## 🙏 Credits

RoutineHero is powered by:

- [InfiniTime](https://github.com/InfiniTimeOrg/InfiniTime) – Original firmware base
- [LittlevGL (LVGL)](https://lvgl.io/) – Embedded graphics library
- The open-source community ❤️

---

## 📄 License

RoutineHero inherits the [InfiniTime license](https://github.com/InfiniTimeOrg/InfiniTime/blob/develop/LICENSE) (Apache 2.0).