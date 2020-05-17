BUILD_DIR := build

debug:
	@ $(MAKE) -f ghost.make NAME=ghost MODE=debug SOURCE_DIR=src
	@ cp build/ghost ghost

clean:
	@ rm -rf $(BUILD_DIR)
	@ rm -rf gen
	@ rm ghost

ghost:
	@ $(MAKE) -f ghost.make NAME=ghost MODE=release SOURCE_DIR=src
	@ cp build/ghost ghost

.PHONY: clean ghost debug
