# Copyright 2026 meatcar (@meatcar)
# SPDX-License-Identifier: GPL-2.0-or-later

ROOTDIR := $(TEST_PATH)
include $(TEST_PATH)/../rules.mk
POINTING_DEVICE_ENABLE = yes
POINTING_DEVICE_DRIVER = custom
MOUSEKEY_ENABLE = yes
EEPROM_DRIVER = transient

SRC += keyboards/ploopyco/ploopyco.c
SRC += pointing_device_accel.c .build/adept_modules/community_modules.c
VPATH += keyboards/ploopyco keyboards/ploopyco/common $(QMK_USERSPACE)/modules/drashna/pointing_device_accel .build/adept_modules
OPT_DEFS += -DQMK_KEYBOARD_H=\"adept.h\" -DCOMMUNITY_MODULES_ENABLE
CFLAGS += -fsanitize=address,undefined -fno-sanitize-recover=all
CXXFLAGS += -fsanitize=address,undefined -fno-sanitize-recover=all
LDFLAGS += -fsanitize=address,undefined -lm

# NOTE: Use VIA's real EEPROM keymap lookup, not TestFixture's replacement.
$(TEST_OBJ)/$(TEST_OUTPUT)/tests/test_common/test_fixture.o: FILE_SPECIFIC_CFLAGS += -Dkeymap_key_to_keycode=fixture_keymap_key_to_keycode
