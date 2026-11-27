include ../../sdk/p4linux.mk

TARGET := $(BUILD_DIR)/p4hello

.PHONY: all install clean

all: $(TARGET)

$(TARGET): main.c ../../sdk/p4linux.mk Makefile
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

install: $(TARGET)
	install -D -m 755 $(TARGET) $(INSTALL_DIR)/p4hello

clean:
	rm -f $(TARGET)
	rm -f $(INSTALL_DIR)/p4hello

