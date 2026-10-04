# Ploopy Adept controls

This keymap keeps the stock button positions and pointer orientation. It adds
single-button tap/hold scrolling, two held chords, and saved VIA settings.

## Buttons and scrolling

| Button | Action |
| ------ | ------ |
| Top left left | Back |
| Top left | Forward |
| Top right | Tap to toggle scrolling; hold to scroll |
| Top right right | Right click |
| Bottom left | Left click |
| Bottom right | Middle click |

Scrolling starts as soon as you press its button. Releasing before the tap cutoff,
without moving the ball, latches scrolling on. Moving the ball or holding for at
least the cutoff makes it momentary. If scrolling was already latched, pressing
the button clears the latch; it still scrolls while held and stops on release.

Fractional scroll movement accumulates instead of being discarded. Pointer motion
stops while scrolling, but mouse buttons still work.

## Held chords

| Chord | Action |
| ----- | ------ |
| Back + right click | Toggle normal layer 0 / macOS layer 1 |
| Forward + right click | Cycle the five configured DPI presets |

Normal scrolling supports both axes. The macOS layer suppresses horizontal
scrolling, even when the ball moves diagonally; ordinary pointer motion is unchanged.
Both layers start with the same button mapping. This is a manual mode switch, not OS detection.

Press the two chord buttons within 50 ms, then hold both for 200 ms. A recognized
chord consumes their ordinary clicks and fires once until released. Releasing a
recognized chord early does nothing. Pressing the buttons too far apart sends
ordinary clicks instead. Back, Forward, and right-click have a short recognition
delay when used separately. Chords follow the button assignments on VIA layer 0.

## VIA settings

Flash `ploopyco_madromys_rev1_001_meatcar.uf2`. Hold bottom-left while reconnecting
to enter the bootloader and reset EEPROM. This first installation resets old VIA
mappings because it adds saved settings storage. Back up mappings before flashing.

In [usevia.app](https://usevia.app), enable the Design tab in Settings. Leave
**Use V2 definitions** off, then load [via.json](via.json) in Design and authorize
the Adept. In Settings, choose **Slider Mode → Slider & Input Field** for exact numbers.
The matching firmware and JSON are both required; the public stock definition
does not contain these menus.

| Menu | Settings and defaults |
| ---- | --------------------- |
| DPI | Active preset; five presets: 600, 900, 1200, 1600, 2400 DPI |
| Scrolling | Horizontal/vertical divisors: 8 each; direction reversal: off |
| Advanced | Tap cutoff: 200 ms; chord press window: 50 ms; chord hold: 200 ms |

DPI sliders use units of 100 DPI: a value of 9 means 900 DPI. The range is
100–12000 DPI in 100-DPI steps. Scroll divisors range from 1–64; higher values
scroll more slowly. Direction reversal affects scrolling, not pointer movement.

Settings take effect immediately. VIA's save command persists them on the device.
The DPI chord uses the editable presets and saves its active preset. Reconnecting
preserves settings and DPI, but starts in the normal layer with scrolling off.
Hold bottom-left during connection to reset all saved settings and mappings.

## Build and test

From the repository root:

```sh
nix develop
make test-adept -j4
qmk compile -kb ploopyco/madromys/rev1_001 -km meatcar
```

The tests use QMK's native GoogleTest build and fixtures. They compile the actual
keymap, Ploopy implementation, combo engine, mouse-key handling, and VIA command
handler. QMK supplies the simulated matrix, clock, sensor motion, host reports,
and RAM-backed EEPROM. Sensor DPI read/write callbacks are supplied by the tests.
No test changes are needed inside the firmware checkout.

QMK 0.33.8 does not discover tests in external userspace. The Makefile target
passes this directory to its native test build and runs the resulting executable.
The fixture's keymap override is renamed so VIA's real EEPROM mappings are used.
The six Adept buttons occupy the first row of QMK's larger native test matrix.
AddressSanitizer and UndefinedBehaviorSanitizer check the native test executable.

Run one test, or repeat the suite in shuffled order:

```sh
make test-adept TEST_ARGS='--gtest_filter=Adept.ViaTimingSettingsControlTapWindowAndHold'
make test-adept TEST_ARGS='--gtest_shuffle --gtest_repeat=20'
```

The tests cover tap/hold boundaries, both chord orders, click suppression,
fractional scrolling, independent axis settings, VIA limits and remappings,
saved settings, and simulated reconnects. After flashing, check button feel,
VIA connectivity, and persistence across a physical power cycle on the Adept.

## Sources and attribution

- [Ryan Heisler's Adept customizations](https://blog.ryanheisler.com/blog/ploopy-adept/customize-ploopy-adept.html)
  inspired combined momentary/toggled scrolling and the selectable vertical-only
  macOS mode. His article's code was not copied. The macOS diagnosis is his report,
  not a claim that every macOS version needs this workaround.
- [plodah/ploopy_viamenus](https://github.com/plodah/ploopy_viamenus/blob/main/readme-resources/FEATURES.md)
  inspired editable DPI/scrolling menus. Its tap dance uses double-tap toggling;
  this keymap deliberately uses single-tap toggling. Its code and menu definitions
  were not imported.
- [QMK 0.33.8 stock Adept keymap](https://github.com/qmk/qmk_firmware/blob/0.33.8/keyboards/ploopyco/madromys/keymaps/default/keymap.c)
  supplies the six-button layout retained in `keymap.c`. Its copyright notices for
  Colin Lam, Christopher Courtney, and Sunjun Kim remain in that file. The header
  records the custom changes and date.
- [QMK Adept hardware layout](https://github.com/qmk/qmk_firmware/blob/0.33.8/keyboards/ploopyco/madromys/info.json)
  supplies the physical positions and matrix coordinates adapted in `via.json`
  and `tests/adept.h`, plus the eight VIA layers used by the tests.
  That definition's custom menus were written for this keymap. The test sensor's
  Y-axis inversion matches the [Adept hardware configuration](https://github.com/qmk/qmk_firmware/blob/0.33.8/keyboards/ploopyco/madromys/config.h).
- [QMK's Ploopy implementation](https://github.com/qmk/qmk_firmware/blob/0.33.8/keyboards/ploopyco/ploopyco.c)
  supplies the live DPI array, active-preset storage, and cycle function used here.
  The [QMK combo engine](https://github.com/qmk/qmk_firmware/blob/0.33.8/quantum/process_keycode/process_combo.c)
  handles chord recognition. Tests compile both upstream sources without copying
  or modifying them; their copyright and license notices remain intact.
- [QMK's native test framework](https://github.com/qmk/qmk_firmware/tree/0.33.8/tests/test_common)
  supplies the GoogleTest fixtures and simulated inputs/reports used in
  `tests/test_adept.cpp`. These upstream files retain their notices and licenses.
- [VIA custom UI documentation](https://www.caniusevia.com/docs/custom_ui) defines
  the menu bindings and command protocol. [Ploopy's programming guide](https://ploopyco.github.io/adept-trackball/appendices/programming/)
  documents the bootloader and recovery procedure.

The firmware changes, test sources, and VIA definition are Copyright 2026
meatcar and licensed under GPL-2.0-or-later, consistent with the retained QMK code.
See the repository [license](../../../../../LICENSE). Source links above distinguish
retained/adapted material from inspiration.
