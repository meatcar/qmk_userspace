# QMK userspace

Personal [QMK userspace](https://docs.qmk.fm/newbs_external_userspace) for
Leopold FC660C controllers and a Ploopy Adept trackball.

| Device | Keymap | Configuration |
| ------ | ------ | ------------- |
| `fc660c` | [`meatcar`](keyboards/fc660c/keymaps/meatcar/README.md) | Hasu controller, ATmega32U4; fixed keyboard map without VIA |
| `cipulot/ec_660c` | `meatcar` | Cipulot controller, STM32F401; keyboard map with VIA and EC settings |
| `ploopyco/madromys/rev1_001` | [`meatcar`](keyboards/ploopyco/madromys/keymaps/meatcar/README.md) | Ploopy Adept, RP2040; tap/hold scrolling, straightening, DPI and VIA settings |
| `fc660c` | [`via`](keyboards/fc660c/keymaps/via/README.md) | Hasu controller; stock-style defaults with VIA remapping, built separately |

The keyboard `meatcar` maps share the ANSI typing layout and Fn navigation
bindings. Fn sits between right Alt and right Ctrl; tap it for Print Screen or
hold it for layer 1. The EC660C also has Fn + grave for the bootloader, VIA
remapping, and consumer media-key reports. The Hasu `meatcar` build disables VIA
and consumer media-key reports. Its separate `via` map has different defaults.

## Setup and build

The Nix development shell provides QMK and its toolchains on x86-64 Linux. For a
fresh checkout, create the separate firmware clone and configure QMK once:

```sh
nix develop
git clone https://github.com/qmk/qmk_firmware.git qmk_firmware
./scripts/sync-qmk
qmk config user.qmk_home="$(realpath qmk_firmware)"
qmk config user.overlay_dir="$(realpath .)"
```

With setup complete, build from the repository root:

```sh
nix develop
qmk userspace-compile
# Or build a single target:
qmk compile -kb fc660c -km meatcar
qmk compile -kb cipulot/ec_660c -km meatcar
qmk compile -kb ploopyco/madromys/rev1_001 -km meatcar
# Optional Hasu VIA map, not registered in qmk.json:
qmk compile -kb fc660c -km via
```

[qmk.json](qmk.json) registers the three `meatcar` targets for local batch builds
and CI. Firmware outputs appear in the repository root and are ignored by VCS.

## Tests

The Adept has native QMK GoogleTest coverage with AddressSanitizer and
UndefinedBehaviorSanitizer. After setup, run from the repository root:

```sh
nix develop
make test-adept -j4
```

See the [Adept guide](keyboards/ploopyco/madromys/keymaps/meatcar/README.md#build-and-test)
for filtered and repeated runs. There is no userspace test suite for the keyboard
maps; check their builds and test their behavior on hardware.

## Flash

### Cipulot EC660C

The EC660C uses `stm32-dfu`. Hold Fn and press grave, or short the exposed reset
pads on the PCB, to enter the bootloader. Then:

```sh
qmk flash -kb cipulot/ec_660c -km meatcar
# Or flash the built .bin manually:
sudo dfu-util -a 0 -d 0483:df11 -s 0x08000000:leave -D cipulot_ec_660c_meatcar.bin
```

After an EEPROM reset, run noise-floor and bottom-out calibration in
[usevia.app](https://usevia.app). `VIA_ENABLE = yes` exposes the Cipulot
calibration, actuation, and rapid-trigger menus.

### Hasu FC660C

The Hasu controller uses `atmel-dfu` and `.hex` firmware. Enter its bootloader
with the controller's reset button, then run:

```sh
qmk flash -kb fc660c -km meatcar
# Or use -km via for the separate VIA map.
```

### Ploopy Adept

The Adept is called `madromys` in QMK. Its `meatcar` keymap keeps the stock button
positions, adds tap/hold scrolling, selectable continuous straightening, and
mode/DPI chords, and exposes saved DPI, scrolling, and timing settings in VIA. See the
[feature guide and source attribution](keyboards/ploopyco/madromys/keymaps/meatcar/README.md)
for controls and loading the matching VIA definition.

To flash, unplug the Adept, hold the bottom-left button, and plug it back in.
Copy `ploopyco_madromys_rev1_001_meatcar.uf2` to the USB drive that appears.
The Adept installs the firmware and restarts automatically. Holding bottom-left
also resets saved settings and VIA mappings; back them up first. See the
[Adept programming guide](https://ploopyco.github.io/adept-trackball/appendices/programming/)
for bootloader recovery instructions.

## Firmware version

`qmk_firmware/` is a separate local clone, not part of this userspace repository.
The QMK version is pinned in
[.github/workflows/build_binaries.yaml](.github/workflows/build_binaries.yaml)
under `qmk_ref`. That pin is the source of truth for CI and `scripts/sync-qmk`.
[Renovate](renovate.json) opens PRs updating QMK and the Nix inputs. After a QMK
version bump, sync the local clone:

```sh
./scripts/sync-qmk
```
