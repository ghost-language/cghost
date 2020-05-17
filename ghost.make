CFLAGS := -std=c99 -Wall -Wextra -Werror -Wno-unused-parameter -fno-strict-aliasing \
          -Wshadow -Wunused-function -Wunused-macros -fno-strict-aliasing
LFLAGS := -lm

ifeq ($(MODE),debug)
	CFLAGS += -O0 -DDEBUG -g
	BUILD_DIR := build/debug
else
	CFLAGS += -O3 -flto
	BUILD_DIR := build/release
endif

HEADERS := $(wildcard $(SOURCE_DIR)/*.h) $(wildcard $(SOURCE_DIR)/modules/*.h) $(wildcard $(SOURCE_DIR)/values/*.h) $(wildcard $(SOURCE_DIR)/vendor/*.h)
SOURCES := $(wildcard $(SOURCE_DIR)/*.c) $(wildcard $(SOURCE_DIR)/modules/*.c) $(wildcard $(SOURCE_DIR)/values/*.c) $(wildcard $(SOURCE_DIR)/vendor/*.c)
OBJECTS := $(addprefix $(BUILD_DIR)/$(NAME)/, $(notdir $(SOURCES:.c=.o)))

build/$(NAME): $(OBJECTS)
	@ printf "%8s %-40s %s\n" $(CC) $@ "$(CFLAGS)"
	@ mkdir -p build
	@ $(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

$(BUILD_DIR)/$(NAME)/%.o: $(SOURCE_DIR)/%.c $(HEADERS)
	@ printf "%8s %-40s %s\n" $(CC) $< "$(CFLAGS)"
	@ mkdir -p $(BUILD_DIR)/$(NAME)
	@ $(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/$(NAME)/%.o: $(SOURCE_DIR)/modules/%.c $(HEADERS)
	@ printf "%8s %-40s %s\n" $(CC) $< "$(CFLAGS)"
	@ mkdir -p $(BUILD_DIR)/$(NAME)
	@ $(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/$(NAME)/%.o: $(SOURCE_DIR)/values/%.c $(HEADERS)
	@ printf "%8s %-40s %s\n" $(CC) $< "$(CFLAGS)"
	@ mkdir -p $(BUILD_DIR)/$(NAME)
	@ $(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/$(NAME)/%.o: $(SOURCE_DIR)/vendor/%.c $(HEADERS)
	@ printf "%8s %-40s %s\n" $(CC) $< "$(CFLAGS)"
	@ mkdir -p $(BUILD_DIR)/$(NAME)
	@ $(CC) -c $(CFLAGS) -o $@ $<

.PHONY: default