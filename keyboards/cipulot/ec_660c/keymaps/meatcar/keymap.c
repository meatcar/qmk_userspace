// Copyright 2026 meatcar (@meatcar)
// SPDX-License-Identifier: GPL-2.0-or-later
//
// Port of the fc660c "meatcar" keymap to the Cipulot EC660C.
// ec_660c LAYOUT has 3 extra positions vs the hasu fc660c, inert on ANSI:
//   row0 second KC_BSPC (split backspace), row2 KC_NUHS (ISO), row3 KC_BSLS (split lshift).
// Bottom-right cluster order on this PCB is RAlt, RCtrl, Fn (hasu was RAlt, Fn, RCtrl).
#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // clang-format off
    [0] = LAYOUT(
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, KC_BSPC, KC_INS,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,          KC_DEL,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_NUHS, KC_ENT,
        KC_LSFT, KC_BSLS, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,          KC_UP,
        KC_LCTL, KC_LGUI, KC_LALT,                   KC_SPC,                   KC_RALT, LT(1, KC_PSCR), KC_RCTL,           KC_LEFT, KC_DOWN, KC_RGHT
    ),
    [1] = LAYOUT(
        QK_BOOT, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_MUTE, KC_MUTE, KC_VOLU,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_F20,          KC_VOLD,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          KC_PGUP,
        _______, _______, _______,                   _______,                   _______, MO(1), _______,                    KC_HOME, KC_PGDN, KC_END
    )
    // clang-format on
};
