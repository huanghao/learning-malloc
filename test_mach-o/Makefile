CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
TARGETS = mach_o_parser symbol_dumper
SOURCES = mach_o_parser.c symbol_dumper.c

.PHONY: all clean test

all: $(TARGETS)

mach_o_parser: mach_o_parser.c
	$(CC) $(CFLAGS) -o mach_o_parser mach_o_parser.c

symbol_dumper: symbol_dumper.c
	$(CC) $(CFLAGS) -o symbol_dumper symbol_dumper.c

clean:
	rm -f $(TARGETS)

# 测试示例
test: $(TARGETS)
	@echo "Testing mach_o_parser with simple_example:"
	./mach_o_parser simple_example
	@echo "\nTesting symbol_dumper with simple_example:"
	./symbol_dumper simple_example

# 安装到系统路径（可选）
install: $(TARGETS)
	sudo cp $(TARGETS) /usr/local/bin/

uninstall:
	sudo rm -f /usr/local/bin/mach_o_parser /usr/local/bin/symbol_dumper

help:
	@echo "Available targets:"
	@echo "  all      - Build the Mach-O parser"
	@echo "  clean    - Remove build artifacts"
	@echo "  test     - Run tests with system binaries"
	@echo "  install  - Install to /usr/local/bin"
	@echo "  uninstall- Remove from /usr/local/bin"
	@echo "  help     - Show this help message"