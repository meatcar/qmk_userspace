# Hasu FC660C meatcar keymap

Fixed keyboard map for the Hasu FC660C controller. This is the custom `meatcar`
map, not the stock QMK map or the separate [`via` map](../via/README.md).

- The top-left key sends grave instead of Escape.
- Fn sits between right Alt and right Ctrl. Tap it for Print Screen; hold it for
  layer 1.
- Fn + 1 through Fn + = sends F1 through F12.
- Fn + arrows sends Home, Page Down, End, and Page Up.
- Fn + backslash sends F20.

VIA, mouse keys, and consumer media-key reports are disabled in
[rules.mk](rules.mk). Mute and volume keycodes remain in [keymap.c](keymap.c), but
this build does not send their consumer reports. The Cipulot EC660C port shares
the typing and navigation bindings, but enables VIA and media reports and adds
Fn + grave for the bootloader.

After the [repository setup](../../../../README.md#setup-and-build), build from
the repository root:

```sh
nix develop
qmk compile -kb fc660c -km meatcar
```

The output is `fc660c_meatcar.hex`. See the
[Hasu flashing instructions](../../../../README.md#hasu-fc660c) to install it.
