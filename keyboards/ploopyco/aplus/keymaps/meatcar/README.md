# Ploopy A+ controls

This keymap is Ploopy's stock A+ firmware: the same buttons, knobs, gestures,
layers, lighting, and saved options. It starts as a copy of the `default` keymap
from Ploopy's [`ploopyco/a+` branch at ed6bc42](https://github.com/ploopyco/qmk_firmware/tree/ed6bc4231cc0c39f5f4c0ff82f0d5ad5d8ef7e6e/keyboards/ploopyco/aplus/keymaps/default),
ready to customize. The build target is `ploopyco/aplus/rev2_001:meatcar`.

## Buttons

| Button | Navigation layer | Control layer |
| ------ | ---------------- | ------------- |
| Top left left | Back | Cycle DPI; blinks N times for preset N |
| Top left | Forward | Cycle LED brightness |
| Top right | Hold to scroll | Toggle vertical knob: scroll / up-down arrows |
| Top right right | Right click | Toggle horizontal knob: scroll / left-right arrows |
| Bottom left | Left click | Toggle left-hand mode |
| Bottom right | Middle click | Toggle drag scroll: hold / tap to latch |
| Left knob press | Hold for gestures | Back to navigation |
| Right knob press | Toggle control layer | Back to navigation |

Left-hand mode mirrors the buttons and swaps the knobs. The LEDs change colour
with the layer and while a gesture is held, and blink to confirm an option
change. Options and brightness persist across restarts.

## Knobs

The left knob scrolls vertically and the right knob horizontally. Linux and
Windows get high-resolution wheel reports; macOS and unknown hosts get stepped
scrolling. On the control layer both knobs change volume.

## Gestures

Hold the left knob and flick the ball:

| Direction | Action |
| --------- | ------ |
| Right | Next desktop |
| Down-right | Paste |
| Down | Copy |
| Down-left | Cut |
| Left | Previous desktop |
| Up-left | Undo |
| Up | Play/pause |
| Up-right | Redo |

Desktop switching and redo send OS-specific shortcuts (Linux: Ctrl+Alt+arrow).
Ploopy's [keymap sheet](https://ploopyco.github.io/aplus-trackball/appendices/manual/)
shows the same layout.

## Build and flash

QMK 0.33.8 does not include the A+ yet. [`scripts/fetch-aplus`](../../../../../scripts/fetch-aplus)
copies the keyboard definition and knob driver from Ploopy's fork into
`qmk_firmware/`; `scripts/sync-qmk` and CI run it before building. The shared
`ploopyco.c` stays at the QMK pin, so the Adept build is unaffected. Ploopy's
branch changes that file to call keymap hooks after its own drag-scroll and DPI
handling; the only visible difference here is which wins if the ball drag-scrolls
while a knob turns.

To flash, unplug the A+, hold the bottom-left button, and plug it back in. Copy
`ploopyco_aplus_rev2_001_meatcar.uf2` to the USB drive that appears. Holding
bottom-left also resets saved options and VIA mappings. VIA needs Ploopy's
[A+ definition](https://ploopyco.github.io/aplus-trackball/appendices/via/).
