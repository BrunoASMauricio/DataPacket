# Used tools
CC := gcc

# Relevant paths
SRC_DIR  := ./src
INC_DIRS := ./inc ./data
OBJ_DIR  := ./obj
EXE_DIR  := ./exe

# Targets and sources
SOURCES :=  $(shell $(COMMAND) find $(SRC_DIR) -name "*.c*")
# get obj/<>.o from src/<>.c
TARGETS := $(subst $(SRC_DIR),$(OBJ_DIR),$(SOURCES) )
# TARGETS := $(subst .cpp,.o,$(TARGETS) )
TARGETS := $(subst .c,.o,$(TARGETS) )

TARGET := $(EXE_DIR)/no_template.exe

# Main flags
LDFLAGS     := -g3
CFLAGS      += -g3
CFLAGS      += -Wmissing-declarations -Wmissing-parameter-type \
               -Wmissing-prototypes -Wbad-function-cast        \
               -Wold-style-definition -Wstrict-prototypes      \
               -Wpointer-sign -Wextra -Wall

SPACE := ${null} ${null}
CFLAGS  += -I$(SRC_DIR) -I$(subst ${SPACE}, -I,$(INC_DIRS))
CFLAGS  += -static -O0

ifdef DEFS
COMMA = ,
CFLAGS += -D $(subst ${COMMA}, -D ,$(DEFS) )
endif

# No defaults
.SUFFIXES:
.PHONY: clean build run debug memory all

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c*
	$(CC) $(CFLAGS) -c $< -o $@

all: run

clean:
	find $(OBJ_DIR) -name "*.o" -exec rm "{}" \;
	find $(EXE_DIR) -name "*.exe" -exec rm "{}" \;

build: clean $(TARGETS)
	$(CC) $(LDFLAGS) $(TARGETS) -o $(TARGET)

run: build $(TARGET)
	$(TARGET)

debug: build $(TARGET)
	gdb $(TARGET)

memory: build $(TARGET)
	valgrind -s --show-leak-kinds=all --leak-check=full --track-origins=yes $(TARGET)
