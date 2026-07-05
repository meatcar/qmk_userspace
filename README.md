# fc660c-hasu

Personal [QMK Userspace](https://docs.qmk.fm/newbs_external_userspace) with my keymaps for
two controllers living in a Leopold FC660C shell.

| Keyboard          | Keymap    | Notes                                              |
| ----------------- | --------- | -------------------------------------------------- |
| `fc660c`          | `meatcar` | Hasu alt controller (ATmega32U4). Retired cable.   |
| `fc660c`          | `via`     | Stock VIA keymap for the hasu board.               |
| `cipulot/ec_660c` | `meatcar` | Cipulot EC660C (STM32F401, EC/Topre). Current.     |

The two `meatcar` keymaps are behavior-identical per physical key; only the extra ANSI-inert
split positions on the EC660C differ.

## Build

```sh
nix develop                                     # qmk CLI + tools
qmk config user.overlay_dir="$(realpath .)"     # first time only
qmk userspace-compile                           # all targets in qmk.json
# or a single target:
qmk compile -kb cipulot/ec_660c -km meatcar
```

## Flash

The EC660C is `stm32-dfu`. Tap RESET on the mainboard to enter the bootloader, then:

```sh
qmk flash -kb cipulot/ec_660c -km meatcar
# or manually with the built .bin:
sudo dfu-util -a 0 -d 0483:df11 -s 0x08000000:leave -D cipulot_ec_660c_meatcar.bin
```

EC boards need calibration after a flash (EEPROM reset). Run noise-floor + bottom-out in
[usevia.app](https://usevia.app); `VIA_ENABLE = yes` keeps that menu available.

The hasu `fc660c` is `atmel-dfu` and flashes `.hex` with `dfu-programmer` / `qmk flash`.

## qmk_firmware pin

`qmk_firmware/` is a local, untracked clone. The build version is pinned in
`.github/workflows/build_binaries.yaml` (`qmk_ref`) as the single source of truth.
[Renovate](renovate.json) opens PRs bumping it (and refreshing `flake.lock`). After a bump,
sync the local clone:

```sh
./scripts/sync-qmk
```
