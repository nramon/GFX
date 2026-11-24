# Build and run the demos on Linux using Open Watcom (cross-compiling to 16-bit
# DOS) and DOSBox.
#
#   make toolchain    Download Open Watcom (~130 MB) and shellcheck into .toolchain/.
#   make              Build all demos into build/.
#   make fade         Build a demo and run it in DOSBox (same as make run-fade).
#   make build/fade.exe   Build a single demo without running it.
#   make test         Run the Python tests (pytest).
#   make lint         Lint Python, shell and C (fails on problems).
#   make analyze      Extra static analysis of the C code (informational).
#   make clean        Remove build output.
#
# From DOS, build with Turbo C++ instead: run MAKE inside a demo directory.
# The Borland-only names used by the sources are mapped to their Open Watcom
# equivalents by compat/watcom/alloc.h, so the sources are built unmodified.

OW_VERSION ?= 2026-11-01-Build
OW_URL     ?= https://github.com/open-watcom/open-watcom-v2/releases/download/$(OW_VERSION)/open-watcom-2_0-c-linux-x64
OW_SHA256  ?= 9d9d823c2ee040a38484ad74ad4a594e904672184e4e8fff84b3bac7090c9f6c

SC_VERSION ?= v0.11.0
SC_URL     ?= https://github.com/koalaman/shellcheck/releases/download/$(SC_VERSION)/shellcheck-$(SC_VERSION).linux.x86_64.tar.xz
SC_SHA256  ?= 8c3be12b05d5c177a04c29e3c78ce89ac86f1595681cab149b65b97c4e227198

TOOLCHAIN  := $(CURDIR)/.toolchain
SHELLCHECK ?= $(TOOLCHAIN)/shellcheck-$(SC_VERSION)/shellcheck
WATCOM     ?= $(TOOLCHAIN)/watcom
WCC        := $(WATCOM)/binl64/wcc
DOSBOX     ?= dosbox
DEMOS      := fade stars
RUNS       := $(addprefix run-,$(DEMOS))

# 16-bit DOS, small memory model, quiet. Use the compat shim and the VGA library.
CFLAGS     := -zq -bt=dos -ms -Icompat/watcom -Ivga
# The default Watcom stack is too small for these programs.
LDFLAGS    := option quiet,stack=16k system dos

export WATCOM
export INCLUDE := $(WATCOM)/h
export PATH    := $(WATCOM)/binl64:$(PATH)

.PHONY: all clean toolchain check $(DEMOS) $(RUNS) \
        test lint lint-py lint-sh lint-c analyze

C_SOURCES  := vga/vga.c fade/fade.c stars/stars.c
# cppcheck does not know the Borland extensions: hide far pointers and asm blocks.
CPPCHECK   := cppcheck -q -Ivga -Dfar= -D__far= -Dasm= --suppress=missingIncludeSystem

all: $(addprefix build/,$(addsuffix .exe,$(DEMOS)))

# Download (if needed), verify and unpack the parts of Open Watcom we use. The
# installer is a zip file behind an ELF stub, so it is unpacked with 7z.
toolchain: $(WCC) $(SHELLCHECK)

$(WCC):
	@command -v 7z >/dev/null || { echo "7z is required (install p7zip-full)."; exit 1; }
	mkdir -p "$(TOOLCHAIN)"
	test -f "$(TOOLCHAIN)/ow-installer" || \
		curl -fL --retry 3 -o "$(TOOLCHAIN)/ow-installer" "$(OW_URL)"
	echo "$(OW_SHA256)  $(TOOLCHAIN)/ow-installer" | sha256sum -c -
	7z x -y -bso0 -bsp0 -o"$(WATCOM)" "$(TOOLCHAIN)/ow-installer" 'binl64/*' 'h/*' 'lib286/*'
	chmod +x "$(WATCOM)"/binl64/*

$(SHELLCHECK):
	mkdir -p "$(TOOLCHAIN)"
	curl -fL --retry 3 -o "$(TOOLCHAIN)/shellcheck.tar.xz" "$(SC_URL)"
	echo "$(SC_SHA256)  $(TOOLCHAIN)/shellcheck.tar.xz" | sha256sum -c -
	tar -xJf "$(TOOLCHAIN)/shellcheck.tar.xz" -C "$(TOOLCHAIN)"
	rm -f "$(TOOLCHAIN)/shellcheck.tar.xz"

check:
	@test -x "$(WATCOM)/binl64/wcc" || { \
		echo "Open Watcom not found in '$(WATCOM)'. Run 'make toolchain'."; exit 1; }

# 'make fade' and 'make stars' build and run the demo, like 'make run-fade'.
$(DEMOS): %: run-%

build:
	mkdir -p build

build/vga.obj: vga/vga.c vga/vga.h compat/watcom/alloc.h | check build
	wcc $(CFLAGS) -fo=$@ $<

build/fade.obj:  fade/fade.c   fade/fade.h   vga/vga.h compat/watcom/alloc.h | check build
	wcc $(CFLAGS) -fo=$@ $<

build/stars.obj: stars/stars.c stars/stars.h vga/vga.h compat/watcom/alloc.h | check build
	wcc $(CFLAGS) -fo=$@ $<

build/fade.exe build/stars.exe: build/%.exe: build/%.obj build/vga.obj
	wlink $(LDFLAGS) name $@ file build/$*.obj file build/vga.obj

# Run a demo from its own directory, since it loads its data files from there.
$(RUNS): run-%: build/%.exe
	$(DOSBOX) -noautoexec \
		-c "mount p $(CURDIR)" -c "p:" -c "cd $*" -c "..\build\$*.exe" -c "exit"

test:
	python3 -m pytest -q tests

lint: lint-py lint-sh lint-c

lint-py:
	flake8 util tests

# Prefer the shellcheck from 'make toolchain', then one in the PATH, and
# otherwise only check the syntax.
lint-sh:
	@if [ -x "$(SHELLCHECK)" ]; then \
		"$(SHELLCHECK)" util/*.sh; \
	elif command -v shellcheck >/dev/null; then \
		shellcheck util/*.sh; \
	else \
		echo "shellcheck not found (run 'make toolchain'), only checking syntax."; \
		for f in util/*.sh; do bash -n "$$f" || exit 1; done; \
	fi

# Real problems only; style findings are reported by 'make analyze'.
lint-c:
	$(CPPCHECK) --enable=warning,portability --error-exitcode=1 $(C_SOURCES)

# Style findings and the compiler's own maximum warning level (syntax check
# only). These never fail the build: the sources predate them.
analyze: check
	-$(CPPCHECK) --enable=style $(C_SOURCES)
	-for f in $(C_SOURCES); do wcc $(CFLAGS) -wx -zs $$f; done

clean:
	rm -rf build
