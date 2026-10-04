// Copyright 2023 Colin Lam (Ploopy Corporation)
// Copyright 2020 Christopher Courtney (@drashna)
// Copyright 2019 Sunjun Kim
// Copyright 2026 meatcar (@meatcar)
// SPDX-License-Identifier: GPL-2.0-or-later
// NOTE: Stock layout retained; custom behavior added 2026-10-04. Sources: README.md.

#include QMK_KEYBOARD_H

enum layers { NORMAL, MACOS };

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [NORMAL] = LAYOUT(MS_BTN4, MS_BTN5, DRAG_SCROLL, MS_BTN2, MS_BTN1, MS_BTN3),
    [MACOS] = LAYOUT(MS_BTN4, MS_BTN5, DRAG_SCROLL, MS_BTN2, MS_BTN1, MS_BTN3)
};

enum chords { SWITCH_MODE, CYCLE_DPI };
const uint16_t PROGMEM mode_chord[] = {MS_BTN4, MS_BTN2, COMBO_END};
const uint16_t PROGMEM dpi_chord[] = {MS_BTN5, MS_BTN2, COMBO_END};
combo_t key_combos[] = {
    [SWITCH_MODE] = COMBO_ACTION(mode_chord),
    [CYCLE_DPI] = COMBO_ACTION(dpi_chord)
};

typedef struct {
    uint16_t dpi[5];
    uint16_t tap_ms, chord_window_ms, chord_hold_ms;
    uint8_t divisor_h, divisor_v;
    uint8_t invert_h, invert_v;
} adept_settings_t;

_Static_assert(sizeof(adept_settings_t) == EECONFIG_USER_DATA_SIZE, "Update EEPROM size and version with the settings layout");

enum setting_ids {
    ACTIVE_DPI = 1, DPI_FIRST, DPI_LAST = DPI_FIRST + 4,
    DIVISOR_H, DIVISOR_V, INVERT_H, INVERT_V, TAP_MS, CHORD_WINDOW_MS, CHORD_HOLD_MS
};

static const adept_settings_t defaults = {
    .dpi = {600, 900, 1200, 1600, 2400},
    .tap_ms = 200, .chord_window_ms = 50, .chord_hold_ms = 200,
    .divisor_h = 8, .divisor_v = 8
};
static adept_settings_t settings;
static bool scroll_held, scroll_latched, tap_eligible;
static uint32_t scroll_pressed_at;
static int32_t scroll_h, scroll_v;
static uint8_t chord_down, pending_chord;
static uint32_t chord_pressed_at[3];
static bool chord_fired;

static void reset_scroll_remainders(void) {
    scroll_h = scroll_v = 0;
}

void eeconfig_init_user(void) {
    settings = defaults;
    eeconfig_update_user_datablock(&settings, 0, sizeof(settings));
}

static bool settings_valid(void) {
    for (uint8_t i = 0; i < ARRAY_SIZE(settings.dpi); i++) {
        if (settings.dpi[i] < 100 || settings.dpi[i] > 12000 || settings.dpi[i] % 100) return false;
    }
    return settings.divisor_h >= 1 && settings.divisor_h <= 64 &&
           settings.divisor_v >= 1 && settings.divisor_v <= 64 &&
           settings.invert_h <= 1 && settings.invert_v <= 1 &&
           settings.tap_ms >= 50 && settings.tap_ms <= 1000 &&
           settings.chord_window_ms >= 10 && settings.chord_window_ms <= 250 &&
           settings.chord_hold_ms >= 50 && settings.chord_hold_ms <= 2000;
}

static void apply_dpi(void) {
    for (uint8_t i = 0; i < ARRAY_SIZE(settings.dpi); i++) dpi_array[i] = settings.dpi[i];
    pointing_device_set_cpi(dpi_array[keyboard_config.dpi_config]);
}

void keyboard_post_init_user(void) {
    if (!eeconfig_is_user_datablock_valid() ||
        eeconfig_read_user_datablock(&settings, 0, sizeof(settings)) != sizeof(settings) ||
        !settings_valid()) eeconfig_init_user();
    if (keyboard_config.dpi_config >= ARRAY_SIZE(settings.dpi)) keyboard_config.dpi_config = 1;
    apply_dpi();
    scroll_held = scroll_latched = tap_eligible = false;
    chord_down = pending_chord = 0;
    chord_fired = false;
    default_layer_set(1u << NORMAL);
    layer_clear();
}

uint16_t get_combo_term(uint16_t combo_index, combo_t *combo) {
    return settings.chord_window_ms;
}

bool pre_process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!IS_KEYEVENT(record->event)) return true;
    keycode = keymap_key_to_keycode(NORMAL, record->event.key);
    uint8_t index;
    switch (keycode) {
        case MS_BTN4: index = 0; break;
        case MS_BTN5: index = 1; break;
        case MS_BTN2: index = 2; break;
        default: return true;
    }
    if (record->event.pressed) {
        chord_down |= 1u << index;
        chord_pressed_at[index] = timer_read32();
    } else {
        chord_down &= ~(1u << index);
        pending_chord = 0;
    }
    return true;
}

void process_combo_event(uint16_t combo_index, bool pressed) {
    if (pressed) {
        pending_chord = combo_index + 1;
        chord_fired = false;
    } else if (pending_chord == combo_index + 1) {
        pending_chord = 0;
    }
}

void housekeeping_task_user(void) {
    if (!pending_chord || chord_fired) return;
    uint8_t index = pending_chord == SWITCH_MODE + 1 ? 0 : 1;
    uint8_t mask = (1u << index) | (1u << 2);
    if ((chord_down & mask) != mask) return;
    if (timer_elapsed32(chord_pressed_at[index]) < settings.chord_hold_ms ||
        timer_elapsed32(chord_pressed_at[2]) < settings.chord_hold_ms) return;
    chord_fired = true;
    if (pending_chord == SWITCH_MODE + 1) {
        layer_invert(MACOS);
    } else {
        cycle_dpi();
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (keycode != DRAG_SCROLL) return true;
    if (record->event.pressed) {
        tap_eligible = !scroll_latched;
        scroll_latched = false;
        scroll_held = true;
        scroll_pressed_at = timer_read32();
    } else {
        scroll_held = false;
        scroll_latched = tap_eligible && timer_elapsed32(scroll_pressed_at) < settings.tap_ms;
        tap_eligible = false;
    }
    reset_scroll_remainders();
    return false;
}

static int8_t scroll_steps(int32_t *remainder, uint8_t divisor) {
    int32_t steps = *remainder / divisor;
    if (steps > 127) steps = 127;
    if (steps < -127) steps = -127;
    *remainder -= steps * divisor;
    return steps;
}

report_mouse_t pointing_device_task_user(report_mouse_t report) {
    if (!scroll_held && !scroll_latched) return report;
    if (scroll_held && (report.x || report.y)) tap_eligible = false;
    if (!(layer_state & (1u << MACOS))) {
        scroll_h += settings.invert_h ? -(int32_t)report.x : report.x;
        report.h = scroll_steps(&scroll_h, settings.divisor_h);
    } else {
        report.h = 0;
    }
    scroll_v += settings.invert_v ? -(int32_t)report.y : report.y;
    report.v = scroll_steps(&scroll_v, settings.divisor_v);
    report.x = report.y = 0;
    return report;
}

layer_state_t layer_state_set_user(layer_state_t state) {
    reset_scroll_remainders();
    return state;
}

void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    if (!length) return;
    if (length < 2 || data[1] != id_custom_channel) {
        data[0] = id_unhandled;
        return;
    }
    if (data[0] == id_custom_save) {
        eeconfig_update_user_datablock(&settings, 0, sizeof(settings));
        eeconfig_update_kb(keyboard_config.raw);
        return;
    }
    if (length < 3 || (data[0] != id_custom_set_value && data[0] != id_custom_get_value)) {
        data[0] = id_unhandled;
        return;
    }

    uint8_t id = data[2];
    uint8_t *byte = NULL;
    uint16_t *word = NULL;
    uint16_t minimum = 0, maximum = 1;
    bool dpi_preset = id >= DPI_FIRST && id <= DPI_LAST;
    if (dpi_preset) {
        word = &settings.dpi[id - DPI_FIRST];
        minimum = 1;
        maximum = 120;
    } else {
        switch (id) {
            case ACTIVE_DPI: byte = &keyboard_config.dpi_config; maximum = 4; break;
            case DIVISOR_H: byte = &settings.divisor_h; minimum = 1; maximum = 64; break;
            case DIVISOR_V: byte = &settings.divisor_v; minimum = 1; maximum = 64; break;
            case INVERT_H: byte = &settings.invert_h; break;
            case INVERT_V: byte = &settings.invert_v; break;
            case TAP_MS: word = &settings.tap_ms; minimum = 50; maximum = 1000; break;
            case CHORD_WINDOW_MS: word = &settings.chord_window_ms; minimum = 10; maximum = 250; break;
            case CHORD_HOLD_MS: word = &settings.chord_hold_ms; minimum = 50; maximum = 2000; break;
            default: data[0] = id_unhandled; return;
        }
    }
    bool wide = maximum > 255;
    if (length < (wide ? 5 : 4)) {
        data[0] = id_unhandled;
        return;
    }
    if (data[0] == id_custom_get_value) {
        uint16_t value = word ? *word : *byte;
        if (dpi_preset) value /= 100;
        data[3] = wide ? value >> 8 : value;
        if (wide) data[4] = value;
        return;
    }
    uint16_t value = wide ? ((uint16_t)data[3] << 8) | data[4] : data[3];
    if (value < minimum || value > maximum) {
        data[0] = id_unhandled;
        return;
    }
    if (word) *word = dpi_preset ? value * 100 : value;
    else *byte = value;
    if (id <= DPI_LAST) apply_dpi();
    if (id >= DIVISOR_H && id <= INVERT_V) reset_scroll_remainders();
}
