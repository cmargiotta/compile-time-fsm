.PHONY: default
default: test

UTILITY_HPP_FILES := $(wildcard src/utility/*.hpp)

.PHONY: clear
clear:
	@rm -rf build

.PHONY: setup
setup:
	@meson setup build

.PHONY: test
test: setup
	@meson test -C build

.PHONY: single_include
single_include: setup
	@./scripts/amalgamate.scm build/ctfsm.hpp $(UTILITY_HPP_FILES) src/fsm/ctfsm.hpp

# guix time-machine gets the latest channel commits if unspecified
.PHONY: update-env
update-env:
	guix time-machine -C channels.scm -- describe --format=channels >channels-lock.scm
