# Hasu FC660C VIA keymap

VIA-enabled keyboard map for the Hasu FC660C controller, with stock-style
defaults. It is separate from the custom [`meatcar` map](../meatcar/README.md).

The defaults in [keymap.c](keymap.c) put Escape at the top left and a momentary
Fn key after right Ctrl. Fn + Escape sends grave; Fn + 1 through Fn + = sends
F1 through F12. Layer 1 also contains Print Screen, Scroll Lock, Pause, and
navigation bindings. Unlike `meatcar`, tapping Fn does not send Print Screen.

[rules.mk](rules.mk) enables VIA for live remapping in
[usevia.app](https://usevia.app). Saved mappings override these compiled defaults.

This map is not in [qmk.json](../../../../qmk.json), so batch builds and CI do not
build it. After the [repository setup](../../../../README.md#setup-and-build),
build it explicitly from the repository root:

```sh
nix develop
qmk compile -kb fc660c -km via
```

The output is `fc660c_via.hex`. See the
[Hasu flashing instructions](../../../../README.md#hasu-fc660c), using `-km via`.
