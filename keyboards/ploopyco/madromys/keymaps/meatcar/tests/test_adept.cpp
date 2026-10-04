// Copyright 2026 meatcar (@meatcar)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "mouse_report_util.hpp"
#include "test_common.hpp"
#include "test_pointing_device_driver.h"

extern "C" {
#include "adept.h"
#include "raw_hid.h"

static uint16_t sensor_cpi;

uint16_t pointing_device_driver_get_cpi(void) {
    return sensor_cpi;
}

void pointing_device_driver_set_cpi(uint16_t cpi) {
    sensor_cpi = cpi;
}
}

using testing::_;

class Adept : public TestFixture {
   protected:
    KeymapKey left{0, 0, 0, MS_BTN1};
    KeymapKey back{0, 1, 0, MS_BTN4};
    KeymapKey forward{0, 2, 0, MS_BTN5};
    KeymapKey scroll{0, 3, 0, DRAG_SCROLL};
    KeymapKey right{0, 4, 0, MS_BTN2};
    KeymapKey middle{0, 5, 0, MS_BTN3};

    void SetUp() override {
        eeconfig_init_quantum();
        keyboard_init();
        pd_clear_movement();
    }

    void press(KeymapKey key) {
        key.press();
        run_one_scan_loop();
    }

    void release(KeymapKey key) {
        key.release();
        run_one_scan_loop();
    }

    void chord(KeymapKey first, KeymapKey second, unsigned window = 20, unsigned hold = 200) {
        press(first);
        idle_for(window - 1);
        press(second);
        idle_for(hold);
        release(first);
        release(second);
    }

    void move(int16_t x, int16_t y) {
        pd_set_x(x);
        pd_set_y(y);
        run_one_scan_loop();
        pd_clear_movement();
    }

    void set(uint8_t id, uint16_t value, bool wide = false) {
        uint8_t packet[32] = {id_custom_set_value, 0, id};
        packet[3]          = wide ? value >> 8 : value;
        packet[4]          = value;
        raw_hid_receive(packet, sizeof(packet));
        ASSERT_EQ(packet[0], id_custom_set_value);
    }

    uint16_t get(uint8_t id, bool wide = false) {
        uint8_t packet[32] = {id_custom_get_value, 0, id};
        raw_hid_receive(packet, sizeof(packet));
        EXPECT_EQ(packet[0], id_custom_get_value);
        return wide ? (uint16_t(packet[3]) << 8) | packet[4] : packet[3];
    }

    void save() {
        uint8_t packet[32] = {id_custom_save, 0};
        raw_hid_receive(packet, sizeof(packet));
        ASSERT_EQ(packet[0], id_custom_save);
    }
};

TEST_F(Adept, StockLayoutAndDefaults) {
    TestDriver     driver;
    const uint16_t codes[] = {MS_BTN1, MS_BTN4, MS_BTN5, DRAG_SCROLL, MS_BTN2, MS_BTN3};
    for (uint8_t layer = 0; layer < 2; ++layer) {
        for (uint8_t col = 0; col < 6; ++col) {
            EXPECT_EQ(dynamic_keymap_get_keycode(layer, 0, col), codes[col]);
        }
    }
    EXPECT_EQ(get(1), 1);
    EXPECT_EQ(get(2), 6);
    EXPECT_EQ(get(3), 9);
    EXPECT_EQ(get(4), 12);
    EXPECT_EQ(get(5), 16);
    EXPECT_EQ(get(6), 24);
    EXPECT_EQ(get(7), 8);
    EXPECT_EQ(get(8), 8);
    EXPECT_EQ(get(9), 0);
    EXPECT_EQ(get(10), 0);
    EXPECT_EQ(get(11, true), 200);
    EXPECT_EQ(get(12), 50);
    EXPECT_EQ(get(13, true), 200);
    EXPECT_EQ(pointing_device_get_cpi(), 900);
    EXPECT_EQ(layer_state, 0);
}

TEST_F(Adept, ScrollStartsImmediatelyAndMovementMakesItMomentary) {
    TestDriver driver;
    press(scroll);
    EXPECT_MOUSE_REPORT(driver, (0, 0, 3, 5, 0));
    move(24, -40);
    release(scroll);
    EXPECT_MOUSE_REPORT(driver, (16, -32, 0, 0, 0));
    move(16, 32);
}

TEST_F(Adept, StandaloneButtonsRetainTheirOrdinaryActions) {
    TestDriver driver;
    for (const auto &button : {std::make_pair(left, 1), std::make_pair(middle, 4), std::make_pair(back, 8), std::make_pair(forward, 16), std::make_pair(right, 2)}) {
        EXPECT_MOUSE_REPORT(driver, (0, 0, 0, 0, button.second));
        EXPECT_EMPTY_MOUSE_REPORT(driver);
        tap_key(button.first, 10);
        idle_for(60);
        VERIFY_AND_CLEAR(driver);
    }
}

class ScrollTap : public Adept, public testing::WithParamInterface<unsigned> {};

TEST_P(ScrollTap, CutoffAndTimerWrap) {
    TestDriver driver;
    tap_key(scroll, GetParam());
    if (GetParam() < 200) {
        EXPECT_MOUSE_REPORT(driver, (0, 0, 2, 3, 0));
    } else {
        EXPECT_MOUSE_REPORT(driver, (16, 24, 0, 0, 0));
    }
    move(16, -24);
}

INSTANTIATE_TEST_CASE_P(Boundaries, ScrollTap, testing::Values(199u, 200u, 65546u));

TEST_F(Adept, PressingALatchedScrollUnlatchesEvenDuringALongHold) {
    TestDriver driver;
    tap_key(scroll);
    press(scroll);
    idle_for(300);
    EXPECT_MOUSE_REPORT(driver, (0, 0, 1, -2, 0));
    move(8, 16);
    release(scroll);
    EXPECT_MOUSE_REPORT(driver, (16, -24, 0, 0, 0));
    move(16, 24);
}

TEST_F(Adept, FractionalMovementAccumulatesAndCancelsTap) {
    TestDriver driver;
    press(scroll);
    EXPECT_NO_MOUSE_REPORT(driver);
    move(1, -2);
    move(2, -3);
    VERIFY_AND_CLEAR(driver);
    EXPECT_MOUSE_REPORT(driver, (0, 0, 1, 1, 0));
    move(5, -3);
    release(scroll);
    EXPECT_MOUSE_REPORT(driver, (24, 16, 0, 0, 0));
    move(24, -16);
}

TEST_F(Adept, SubStepMovementAloneDoesNotLatch) {
    TestDriver driver;
    press(scroll);
    EXPECT_NO_MOUSE_REPORT(driver);
    move(1, 0);
    release(scroll);
    VERIFY_AND_CLEAR(driver);
    EXPECT_MOUSE_REPORT(driver, (24, 16, 0, 0, 0));
    move(24, -16);
}

TEST_F(Adept, ScrollPreservesOtherButtonsAndClearsRemaindersOnRelease) {
    TestDriver driver;
    EXPECT_MOUSE_REPORT(driver, (0, 0, 0, 0, 1));
    press(left);
    press(scroll);
    EXPECT_MOUSE_REPORT(driver, (0, 0, 0, 1, 1));
    move(3, -8);
    release(scroll);
    press(scroll);
    EXPECT_NO_MOUSE_REPORT(driver);
    move(5, 0);
    VERIFY_AND_CLEAR(driver);
    release(scroll);
    EXPECT_EMPTY_MOUSE_REPORT(driver);
    release(left);
}

class ModeChord : public Adept, public testing::WithParamInterface<bool> {};

TEST_P(ModeChord, RequiresBothHoldsConsumesClicksAndFiresOnce) {
    TestDriver driver;
    EXPECT_NO_MOUSE_REPORT(driver);
    auto first  = GetParam() ? back : right;
    auto second = GetParam() ? right : back;
    press(first);
    idle_for(49);
    press(second);
    idle_for(199);
    EXPECT_EQ(layer_state, 0);
    run_one_scan_loop();
    EXPECT_EQ(layer_state, 2);
    idle_for(1000);
    EXPECT_EQ(layer_state, 2);
    release(first);
    release(second);
    chord(first, second);
    EXPECT_EQ(layer_state, 0);
}

INSTANTIATE_TEST_CASE_P(BothOrders, ModeChord, testing::Bool());

TEST_F(Adept, DistantChordPressesSendOrdinaryClicks) {
    TestDriver          driver;
    testing::InSequence order;
    EXPECT_MOUSE_REPORT(driver, (0, 0, 0, 0, 8));
    EXPECT_MOUSE_REPORT(driver, (0, 0, 0, 0, 10));
    EXPECT_MOUSE_REPORT(driver, (0, 0, 0, 0, 2));
    EXPECT_EMPTY_MOUSE_REPORT(driver);
    chord(back, right, 51, 250);
    EXPECT_EQ(layer_state, 0);
}

TEST_F(Adept, EarlyReleaseConsumesChordWithoutChangingMode) {
    TestDriver driver;
    EXPECT_NO_MOUSE_REPORT(driver);
    chord(back, right, 20, 100);
    idle_for(500);
    EXPECT_EQ(layer_state, 0);
}

TEST_F(Adept, UnrelatedClickDoesNotBypassChordHold) {
    TestDriver driver;
    press(back);
    idle_for(9);
    press(right);
    idle_for(4);
    EXPECT_MOUSE_REPORT(driver, (0, 0, 0, 0, 1));
    press(left);
    idle_for(193);
    EXPECT_EQ(layer_state, 0);
    run_one_scan_loop();
    EXPECT_EQ(layer_state, 0);
    run_one_scan_loop();
    EXPECT_EQ(layer_state, 2);
    EXPECT_EMPTY_MOUSE_REPORT(driver);
    release(left);
    release(right);
    release(back);
}

TEST_F(Adept, MacosModeSuppressesHorizontalScrollNotPointerMotion) {
    TestDriver driver;
    chord(back, right);
    EXPECT_EQ(layer_state, 2);
    press(scroll);
    EXPECT_MOUSE_REPORT(driver, (0, 0, 0, 3, 0));
    move(40, -24);
    release(scroll);
    EXPECT_MOUSE_REPORT(driver, (40, 24, 0, 0, 0));
    move(40, -24);
    chord(right, back);
    press(scroll);
    EXPECT_MOUSE_REPORT(driver, (0, 0, 5, 3, 0));
    move(40, -24);
    release(scroll);
}

TEST_F(Adept, ChangingModeDiscardsFractionalScrollFromThePreviousMode) {
    TestDriver driver;
    tap_key(scroll);
    EXPECT_NO_MOUSE_REPORT(driver);
    move(7, -3);
    chord(back, right);
    move(1, -5);
    chord(right, back);
    move(1, -3);
    VERIFY_AND_CLEAR(driver);
    EXPECT_MOUSE_REPORT(driver, (0, 0, 1, 1, 0));
    move(7, -5);
    tap_key(scroll);
}

TEST_F(Adept, DpiChordUsesEditedPresetsWrapsAndPersists) {
    TestDriver driver;
    EXPECT_NO_MOUSE_REPORT(driver);
    set(1, 4);
    set(2, 17);
    save();
    press(forward);
    idle_for(19);
    press(right);
    idle_for(199);
    EXPECT_EQ(get(1), 4);
    run_one_scan_loop();
    EXPECT_EQ(get(1), 0);
    EXPECT_EQ(pointing_device_get_cpi(), 1700);
    idle_for(1000);
    EXPECT_EQ(get(1), 0);
    release(right);
    release(forward);
    keyboard_init();
    EXPECT_EQ(get(1), 0);
    EXPECT_EQ(pointing_device_get_cpi(), 1700);
    chord(right, forward);
    EXPECT_EQ(get(1), 1);
    EXPECT_EQ(pointing_device_get_cpi(), 900);
}

TEST_F(Adept, ViaSpeedReversalAndDpiApplyImmediately) {
    TestDriver driver;
    set(1, 2);
    set(4, 37);
    EXPECT_EQ(pointing_device_get_cpi(), 3700);
    set(7, 3);
    set(8, 5);
    set(9, 1);
    press(scroll);
    EXPECT_MOUSE_REPORT(driver, (0, 0, -3, 3, 0));
    move(11, -17);
    EXPECT_MOUSE_REPORT(driver, (0, 0, -1, 1, 0));
    move(1, -3);
    set(9, 0);
    set(10, 1);
    EXPECT_MOUSE_REPORT(driver, (0, 0, 3, -2, 0));
    move(9, -10);
    release(scroll);
}

TEST_F(Adept, ViaAcceptsAllAdvertisedSettingLimits) {
    TestDriver     driver;
    const uint16_t limits[][3] = {{1, 0, 4}, {2, 1, 120}, {3, 1, 120}, {4, 1, 120}, {5, 1, 120}, {6, 1, 120}, {7, 1, 64}, {8, 1, 64}, {9, 0, 1}, {10, 0, 1}, {11, 50, 1000}, {12, 10, 250}, {13, 50, 2000}};
    for (const auto &limit : limits) {
        bool wide = limit[0] == 11 || limit[0] == 13;
        set(limit[0], limit[1], wide);
        EXPECT_EQ(get(limit[0], wide), limit[1]);
        set(limit[0], limit[2], wide);
        EXPECT_EQ(get(limit[0], wide), limit[2]);
    }
    EXPECT_EQ(pointing_device_get_cpi(), 12000);
    save();
    keyboard_init();
    EXPECT_EQ(get(11, true), 1000);
    EXPECT_EQ(get(12), 250);
    EXPECT_EQ(get(13, true), 2000);
}

TEST_F(Adept, ViaRemappingsPersistAndChordsFollowBaseLayerAcrossModes) {
    TestDriver driver;
    uint8_t    remap[32] = {id_dynamic_keymap_set_keycode, 0, 0, 5, uint8_t(MS_BTN4 >> 8), uint8_t(MS_BTN4)};
    raw_hid_receive(remap, sizeof(remap));
    EXPECT_EQ(remap[0], id_dynamic_keymap_set_keycode);
    dynamic_keymap_set_keycode(0, 0, 1, MS_BTN3);
    dynamic_keymap_set_keycode(1, 0, 5, KC_A);
    dynamic_keymap_set_keycode(1, 0, 4, KC_B);
    EXPECT_MOUSE_REPORT(driver, (0, 0, 0, 0, 4));
    EXPECT_EMPTY_MOUSE_REPORT(driver);
    tap_key(back, 10);
    idle_for(60);
    VERIFY_AND_CLEAR(driver);
    EXPECT_NO_MOUSE_REPORT(driver);
    EXPECT_NO_REPORT(driver);
    chord(middle, right);
    EXPECT_EQ(layer_state, 2);
    chord(right, middle);
    EXPECT_EQ(layer_state, 0);
    keyboard_init();
    EXPECT_EQ(dynamic_keymap_get_keycode(0, 0, 5), MS_BTN4);
    EXPECT_EQ(dynamic_keymap_get_keycode(0, 0, 1), MS_BTN3);
    EXPECT_EQ(dynamic_keymap_get_keycode(1, 0, 5), KC_A);
}

TEST_F(Adept, ViaTimingSettingsControlTapWindowAndHold) {
    TestDriver driver;
    set(11, 350, true);
    set(12, 80);
    set(13, 700, true);
    tap_key(scroll, 349);
    EXPECT_MOUSE_REPORT(driver, (0, 0, 2, -3, 0));
    move(16, 24);
    tap_key(scroll);
    tap_key(scroll, 350);
    EXPECT_MOUSE_REPORT(driver, (16, -24, 0, 0, 0));
    move(16, 24);
    press(right);
    idle_for(79);
    press(back);
    idle_for(699);
    EXPECT_EQ(layer_state, 0);
    run_one_scan_loop();
    EXPECT_EQ(layer_state, 2);
    release(right);
    release(back);
    uint8_t packet[32] = {id_custom_get_value, 0, 11};
    raw_hid_receive(packet, sizeof(packet));
    EXPECT_EQ(packet[3], 1);
    EXPECT_EQ(packet[4], 94);
}

TEST_F(Adept, ViaSavePreservesSettingsAndDpiButNotTransientModes) {
    TestDriver driver;
    set(1, 3);
    set(5, 29);
    set(7, 3);
    set(8, 5);
    set(9, 1);
    set(10, 1);
    set(11, 350, true);
    set(12, 80);
    set(13, 700, true);
    save();
    set(1, 0);
    set(5, 42);
    set(7, 17);
    keyboard_init();
    EXPECT_EQ(get(1), 3);
    EXPECT_EQ(get(5), 29);
    EXPECT_EQ(pointing_device_get_cpi(), 2900);
    EXPECT_EQ(get(7), 3);
    EXPECT_EQ(get(8), 5);
    EXPECT_EQ(get(9), 1);
    EXPECT_EQ(get(10), 1);
    EXPECT_EQ(get(11, true), 350);
    EXPECT_EQ(get(12), 80);
    EXPECT_EQ(get(13, true), 700);
    chord(back, right, 20, 700);
    tap_key(scroll);
    keyboard_init();
    EXPECT_EQ(layer_state, 0);
    EXPECT_EQ(default_layer_state, 1);
    EXPECT_MOUSE_REPORT(driver, (16, 24, 0, 0, 0));
    move(16, -24);
}

TEST_F(Adept, InvalidViaValuesAndCorruptSettingsRecoverSafely) {
    TestDriver    driver;
    const uint8_t invalid[][5] = {{7, 0, 1, 5}, {7, 0, 2, 0}, {7, 0, 7, 0}, {7, 0, 8, 65}, {7, 0, 9, 2}, {7, 0, 11, 0, 49}, {7, 0, 12, 251}, {7, 0, 13, 7, 209}, {7, 0, 99}, {7, 1, 1}};
    for (const auto &request : invalid) {
        uint8_t packet[32] = {};
        memcpy(packet, request, sizeof(request));
        raw_hid_receive(packet, sizeof(packet));
        EXPECT_EQ(packet[0], id_unhandled);
    }
    EXPECT_EQ(get(1), 1);
    EXPECT_EQ(get(2), 6);
    EXPECT_EQ(get(7), 8);
    EXPECT_EQ(get(8), 8);
    EXPECT_EQ(get(9), 0);
    EXPECT_EQ(get(11, true), 200);
    EXPECT_EQ(get(12), 50);
    EXPECT_EQ(get(13, true), 200);
    uint8_t corrupt[EECONFIG_USER_DATA_SIZE] = {};
    eeconfig_update_user_datablock(corrupt, 0, sizeof(corrupt));
    keyboard_init();
    EXPECT_EQ(get(3), 9);
    EXPECT_EQ(get(11, true), 200);
    EXPECT_EQ(get(7), 8);
}
