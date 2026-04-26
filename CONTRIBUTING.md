# Contributing

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.3+-blue?logo=espressif&logoColor=white)](https://github.com/espressif/esp-idf)
[![Target](https://img.shields.io/badge/target-ESP32--S3-orange?logo=espressif&logoColor=white)](https://www.espressif.com/en/products/socs/esp32-s3)

---

## Requirements

| Tool | Version | Notes |
|---|---|---|
| **ESP-IDF** | ≥ 5.3 | full install with toolchain |
| **Python** | ≥ 3.10 | bundled with ESP-IDF |
| **Git** | any current | |
| **Pillow** | any | only for `tools/png2lvgl.py` — `pip3 install Pillow` |

---

## Installing ESP-IDF

### Linux

```bash
# system dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install git wget flex bison gperf python3 python3-pip python3-venv \
     cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0

# clone ESP-IDF
mkdir -p ~/esp && cd ~/esp
git clone --recursive --branch v5.4 https://github.com/espressif/esp-idf.git

# install toolchain for ESP32-S3
cd ~/esp/esp-idf
./install.sh esp32s3

# add alias to ~/.bashrc or ~/.zshrc
echo 'alias get_idf=". $HOME/esp/esp-idf/export.sh"' >> ~/.bashrc
source ~/.bashrc
```

Run `get_idf` at the start of every session before working with the project.

**USB port access** — without this `idf.py flash` will fail with permission denied:

```bash
sudo usermod -aG dialout $USER
# log out and back in for the change to take effect
```

Check the port after plugging in the board: `ls /dev/ttyACM*`  
The board uses native USB CDC (ESP32-S3) — it shows up as `/dev/ttyACM0`.

---

### Windows

**Recommended: official installer**

1. Download the **ESP-IDF Tools Installer** from [dl.espressif.com/dl/esp-idf](https://dl.espressif.com/dl/esp-idf)
2. During install select target **ESP32-S3** and version **≥ v5.3**
3. After install, always open projects via the **ESP-IDF CMD** or **ESP-IDF PowerShell** shortcut — these have the environment variables pre-configured

After plugging in the board, check the port in **Device Manager → Ports (COM & LPT)**.  
The board uses native USB CDC — Windows 10/11 detects it automatically as `COMx`, no extra driver needed.

**Alternative: WSL2**

If you prefer a Linux environment, install WSL2 with Ubuntu and follow the Linux steps above.  
To forward USB into WSL, use [usbipd-win](https://github.com/dorssel/usbipd-win):

```powershell
# PowerShell as administrator
usbipd list
usbipd bind --busid <ID>
usbipd attach --wsl --busid <ID>
```

---

## First build

```bash
git clone https://github.com/n02b3rt/esp32s3-waveshare27690-bsp.git
cd esp32s3-waveshare27690-bsp/examples/display_lvgl_demo

idf.py build
```

`sdkconfig.defaults` sets the `esp32s3` target automatically — `idf.py set-target` is not needed.  
If the build passes without errors, your setup is good.

---

## Workflow

### Branching

```bash
git checkout -b feat/what-you-add    # new feature
git checkout -b fix/what-you-fix     # bug fix
git checkout -b docs/what-you-update # documentation
```

Do not commit directly to `main`.

---

### What to change and how to test it

**BSP core** — `src/`, `include/bsp/bsp.h`, `Kconfig`, `priv_include/`

Display driver, touch driver, LVGL init, GPIO. After any change:

```bash
cd examples/display_lvgl_demo
rm -rf build/ managed_components/   # clean state — important when touching Kconfig or CMake
idf.py build
idf.py -p /dev/ttyACM0 flash monitor   # Linux
idf.py -p COM3 flash monitor           # Windows
```

BSP changes **must be tested on real hardware** — there are no unit tests.  
State in the PR which board revision you tested on.

---

**kni_ui** — `kni_ui/src/kni_ui.c`, `kni_ui/include/kni_ui/kni_ui.h`

UI component — splash screen, tabs, card helpers. The demo is its only consumer, so test through it:

```bash
cd examples/display_lvgl_demo
idf.py build && idf.py -p /dev/ttyACM0 flash monitor
```

New public functions must be documented in `kni_ui/README.md`.

---

**Demo** — `examples/display_lvgl_demo/`

The demo should stay a thin example of BSP + kni_ui usage, not a feature showcase.  
Keep `main.c` simple — if logic starts growing, it belongs in kni_ui instead.

---

**Tools** — `tools/png2lvgl.py`

```bash
# run from the repo root
python3 tools/png2lvgl.py some_image.png test_var 160 160
# verify that test_var.c and test_var.h are valid and compile inside the demo
```

---

### What not to commit

`.gitignore` already covers this, but to be explicit — never add:

- `build/` — build artifacts
- `managed_components/` — fetched automatically by idf.py
- `sdkconfig`, `sdkconfig.old` — local build config
- `dependencies.lock` — generated automatically

---

## Code style

- Indent with **4 spaces**, no tabs
- Braces: K&R style (same as ESP-IDF and the rest of the codebase)
- Names: `snake_case` for functions and variables, `UPPER_SNAKE` for macros
- API prefixes: `bsp_*` for BSP, `kni_ui_*` for kni_ui
- Comments only when the *why* is non-obvious — don't describe what the code does

---

## Commit messages

Format: `type: short description` — lowercase, no trailing period

```
feat: add PWM backlight control
fix: correct CST328 reset timing on cold boot
docs: add pin mapping to README
refactor: extract lvgl port config to separate function
chore: bump lvgl dependency to 9.2.2
```

Types: `feat` `fix` `docs` `refactor` `chore`

---

## Pull request checklist

- `idf.py build` passes on a clean `build/`
- Hardware changes tested on a physical board (state which revision)
- New public API documented in the relevant README
- PR description explains what and why

---

## Questions and bugs

[GitHub Issues](https://github.com/n02b3rt/esp32s3-waveshare27690-bsp/issues)
