TARGET:= grammer_parse

CC:= gcc

INCLUDE_DIR:=$(shell pwd)/../../../include
LIBS_DIR:=$(shell pwd)/../../../libs

CFLAGS:= -O1 -I$(INCLUDE_DIR) -I./

LINK_LIBS:= -lbase

.phony: all clean

all: $(TARGET)

%:%.c
	@$(CC) $(CFLAGS) -o $@ $^ -L$(LIBS_DIR) $(LINK_LIBS)

clean:
	@rm -f $(TARGET)

