.SILENT:

MAKEFLAGS += --no-print-directory

QMK_USERSPACE := $(patsubst %/,%,$(dir $(shell realpath "$(lastword $(MAKEFILE_LIST))")))
ifeq ($(QMK_USERSPACE),)
    QMK_USERSPACE := $(shell pwd)
endif

# Determine which qmk cli to use
QMK_BIN := qmk

# Try to determine qmk_firmware from qmk config - no fallback is required as this functionality has existed since qmk_cli 0.0.46
QMK_FIRMWARE_ROOT = $(shell $(QMK_BIN) env QMK_FIRMWARE)
ifeq ($(QMK_FIRMWARE_ROOT),)
    $(error Cannot determine qmk_firmware location. `qmk config -ro user.qmk_home` or QMK_HOME environment variable is not set)
endif

ADEPT_MODULES := $(QMK_FIRMWARE_ROOT)/.build/adept_modules
ADEPT_KEYMAP_JSON := $(QMK_USERSPACE)/keyboards/ploopyco/madromys/keymaps/meatcar/keymap.json
ADEPT_MODULE_METADATA := $(QMK_USERSPACE)/modules/drashna/pointing_device_accel/qmk_module.json

.PHONY: test-adept
test-adept: $(ADEPT_MODULES)/community_modules.h $(ADEPT_MODULES)/community_modules.c $(ADEPT_MODULES)/community_modules_introspection.h $(ADEPT_MODULES)/community_modules_introspection.c
	+$(MAKE) -C $(QMK_FIRMWARE_ROOT) -f builddefs/build_test.mk TEST=adept TEST_OUTPUT=adept TEST_PATH="$(QMK_USERSPACE)/keyboards/ploopyco/madromys/keymaps/meatcar/tests" QMK_USERSPACE="$(QMK_USERSPACE)" FULL_TESTS=adept SILENT=true
	$(QMK_FIRMWARE_ROOT)/.build/test/adept.elf $(TEST_ARGS)

$(ADEPT_MODULES)/community_modules.%: $(ADEPT_KEYMAP_JSON) $(ADEPT_MODULE_METADATA) .github/workflows/build_binaries.yaml
	mkdir -p "$(@D)"
	qmk generate-community-modules-$* -kb ploopyco/madromys/rev1_001 --quiet --output "$@" "$(ADEPT_KEYMAP_JSON)"

$(ADEPT_MODULES)/community_modules_introspection.%: $(ADEPT_KEYMAP_JSON) $(ADEPT_MODULE_METADATA) .github/workflows/build_binaries.yaml
	mkdir -p "$(@D)"
	qmk generate-community-modules-introspection-$* -kb ploopyco/madromys/rev1_001 --quiet --output "$@" "$(ADEPT_KEYMAP_JSON)"

%:
	+$(MAKE) -C $(QMK_FIRMWARE_ROOT) $(MAKECMDGOALS) QMK_USERSPACE=$(QMK_USERSPACE)
