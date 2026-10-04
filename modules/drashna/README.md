# Drashna pointing-device acceleration

The seven files in `pointing_device_accel/` are copied unchanged from
[drashna/qmk_modules at eb0ea42](https://github.com/drashna/qmk_modules/tree/eb0ea42b4a0179b562071dc36eeba4a1c0d10873/pointing_device_accel).
This revision requires community-module API 1.1.0 and works with the repository's
QMK 0.33.8 pin. Later upstream revisions require API 1.1.3 and module EEPROM
support, so the dependency must not be updated independently of QMK compatibility.

The C sources and headers retain their copyright notices for Christopher
Courtney, burkfers, and Wimads and their GPL-2.0-or-later SPDX identifiers.
[LICENSE](LICENSE) is copied unchanged from that upstream revision. The
[upstream README](https://github.com/drashna/qmk_modules/blob/eb0ea42b4a0179b562071dc36eeba4a1c0d10873/pointing_device_accel/README.md)
documents the curve, original authors, runtime APIs, and macOS caveats. Its images
and optional VIA menu assets are not vendored here.

Only the Adept selects this module, through its
[keymap.json](../../keyboards/ploopyco/madromys/keymaps/meatcar/keymap.json).
The keymap's processing gate bypasses acceleration on layer 1 and during held or
latched scrolling. The module's optional VIA implementation is not enabled;
acceleration uses startup defaults without allocating VIA settings storage.
The existing DPI, scrolling, and timing menus and EEPROM layout are unchanged.

See the [Adept guide](../../keyboards/ploopyco/madromys/keymaps/meatcar/README.md#pointer-acceleration)
for the configured behavior and tuning instructions.
