.SILENT:

MAKEFLAGS += --no-print-directory

QMK_USERSPACE := $(patsubst %/,%,$(dir $(shell realpath "$(lastword $(MAKEFILE_LIST))")))
ifeq ($(QMK_USERSPACE),)
    QMK_USERSPACE := $(shell pwd)
endif

QMK_FIRMWARE_ROOT = $(shell qmk config -ro user.qmk_home | cut -d= -f2 | sed -e 's@^None$$@@g')
ifeq ($(QMK_FIRMWARE_ROOT),)
    $(error Cannot determine qmk_firmware location. `qmk config -ro user.qmk_home` is not set)
endif

.PHONY: test-adept
test-adept:
	+$(MAKE) -C $(QMK_FIRMWARE_ROOT) -f builddefs/build_test.mk TEST=adept TEST_OUTPUT=adept TEST_PATH="$(QMK_USERSPACE)/keyboards/ploopyco/madromys/keymaps/meatcar/tests" FULL_TESTS=adept SILENT=true
	$(QMK_FIRMWARE_ROOT)/.build/test/adept.elf $(TEST_ARGS)

%:
	+$(MAKE) -C $(QMK_FIRMWARE_ROOT) $(MAKECMDGOALS) QMK_USERSPACE=$(QMK_USERSPACE)
