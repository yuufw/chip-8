# ---------- Defaults ----------
.DEFAULT_GOAL := sdl
.SUFFIXES:

# ---------- Toolchain ----------
CC      ?= gcc
PKGCONF ?= pkg-config
LD      ?= ld
OBJCOPY ?= objcopy

# ---------- Project paths ----------
SRCDIR  ?= src
HDRDIR  ?= inc
BINDIR  ?= bin
OBJROOT ?= obj
ROMDIR  ?= roms

# ---------- Sources ----------
SRCS := $(wildcard $(SRCDIR)/*.c)
ROM_FILES := $(wildcard $(ROMDIR)/*)
ROM_OBJS  := $(patsubst $(ROMDIR)/%,$(OBJROOT)/roms/%.o,$(ROM_FILES))
ROM_MANIFEST := $(OBJROOT)/roms/roms_manifest.h

# ---------- Verbosity ----------
V ?= 0
ifeq ($(V),0)
  Q := @
else
  Q :=
endif

# ---------- Flag helpers (only add flags supported by compiler/linker) ----------
cc-option  = $(shell $(CC) -Werror $(1) -x c -c -o /dev/null - </dev/null >/dev/null 2>&1 && echo $(1))
cc-options = $(foreach f,$(1),$(call cc-option,$(f)))

ld-option  = $(shell printf 'int main(void){return 0;}\n' | \
  $(CC) -Werror $(1) -x c -o /dev/null - >/dev/null 2>&1 && echo $(1))
ld-options = $(foreach f,$(1),$(call ld-option,$(f)))

# ---------- Base flags ----------
CPPFLAGS ?= -I./$(HDRDIR)
CPPFLAGS += -I$(OBJROOT)/roms
CFLAGS   ?= -std=c11
LDFLAGS  ?=
LDLIBS   ?=

# Auto deps
CFLAGS   += -MMD -MP

# ---------- SDL via pkg-config (preferred) ----------
HAVE_PKGCONF := $(shell command -v $(PKGCONF) >/dev/null 2>&1 && echo yes || echo no)
ifeq ($(HAVE_PKGCONF),yes)
  SDL_PKG_CFLAGS := $(shell $(PKGCONF) --cflags sdl2 SDL2_ttf 2>/dev/null)
  SDL_PKG_LIBS   := $(shell $(PKGCONF) --libs   sdl2 SDL2_ttf 2>/dev/null)
else
  SDL_PKG_CFLAGS :=
  SDL_PKG_LIBS   :=
endif

ifeq ($(strip $(SDL_PKG_LIBS)),)
  SDL_PKG_CFLAGS :=
  SDL_PKG_LIBS   := -lSDL2 -lSDL2_ttf
endif

# ---------- macOS Homebrew SDL fallback ----------
ifeq ($(shell uname),Darwin)
  ifeq ($(strip $(SDL_PKG_CFLAGS)),)
    BREW_PREFIX := $(shell brew --prefix 2>/dev/null)
    ifneq ($(BREW_PREFIX),)
      SDL_PKG_CFLAGS := -I$(BREW_PREFIX)/include -I$(BREW_PREFIX)/include/SDL2
      SDL_PKG_LIBS   := -L$(BREW_PREFIX)/lib -lSDL2 -lSDL2_ttf
    endif
  endif
endif



# ---------- Common flags (embedded in ALL targets) ----------
WERROR ?= 1

COMMON_CFLAGS_RAW := \
  -fstack-protector-strong \
  -fstack-clash-protection \
  -fno-delete-null-pointer-checks \
  -fwrapv \
  -fno-common \
  -fstrict-aliasing \
  -fanalyzer \
  -Wall -Wextra -Wpedantic \
  -Wshadow=global -Wshadow=local \
  -Wbad-function-cast \
  -Wcast-function-type \
  -Wcast-align \
  -Wcast-qual \
  -Wconversion \
  -Warith-conversion \
  -Wsign-conversion \
  -Wuninitialized \
  -Wmaybe-uninitialized \
  -Winit-self \
  -Wclobbered \
  -Wunused-parameter \
  -Wunused-result \
  -Wstrict-prototypes \
  -Wmissing-prototypes \
  -Wmissing-declarations \
  -Wnested-externs \
  -Wunused-function \
  -Wpointer-arith \
  -Waddress \
  -Walloca \
  -Wfree-nonheap-object \
  -Wrestrict \
  -Wnonnull \
  -Wformat=2 \
  -Wformat-overflow=2 \
  -Wformat-truncation=2 \
  -Wformat-signedness \
  -Wformat-y2k \
  -Wmissing-format-attribute \
  -Wformat-security \
  -Werror=format-security \
  -Wunreachable-code \
  -Wsequence-point \
  -Wjump-misses-init \
  -Wstack-usage=8192 \
  -Wfloat-equal \
  -Wdouble-promotion \
  -Wwrite-strings \
  -Wredundant-decls \
  -Wswitch-default \
  -Wswitch-enum \
  -Wduplicated-cond \
  -Wduplicated-branches \
  -Wlogical-op \
  -Wunsafe-loop-optimizations \
  -Wvector-operation-performance \
  -Wvla \
  -Wempty-body \
  -Wdate-time \
  -Winvalid-pch \
  -Wtrigraphs \
  -Wundef \
  -Wsuggest-attribute=format \
  -Wsuggest-attribute=noreturn \
  -Wdeprecated \
  -Wno-deprecated-declarations \
  -Wpacked-bitfield-compat \
  -Wpacked-not-aligned \
  -Waddress-of-packed-member

COMMON_LDFLAGS_RAW := \
  -Wl,--as-needed

COMMON_CFLAGS  := $(call cc-options,$(COMMON_CFLAGS_RAW))
COMMON_LDFLAGS := $(call ld-options,$(COMMON_LDFLAGS_RAW))

ifeq ($(WERROR),1)
  COMMON_CFLAGS += $(call cc-options,-Werror)
endif

CFLAGS  += $(COMMON_CFLAGS)
LDFLAGS += $(COMMON_LDFLAGS)

# ---------- Build modes ----------
MODE_NORMAL_CFLAGS := $(call cc-options,-O2)

MODE_RELEASE_CFLAGS_RAW := \
  -DNDEBUG -Ofast \
  -fno-strict-overflow -fomit-frame-pointer -funroll-loops \
  -fno-exceptions -fno-unwind-tables \
  -fdata-sections -ffunction-sections \
  -g0 -fno-ident -fvisibility=hidden
MODE_RELEASE_CFLAGS := $(call cc-options,$(MODE_RELEASE_CFLAGS_RAW))

MODE_RELEASE_SIZE_CFLAGS_RAW := \
  -DNDEBUG -Os \
  -fno-strict-overflow -fomit-frame-pointer \
  -fno-exceptions -fno-unwind-tables \
  -fdata-sections -ffunction-sections \
  -g0 -fno-ident -fvisibility=hidden
MODE_RELEASE_SIZE_CFLAGS := $(call cc-options,$(MODE_RELEASE_SIZE_CFLAGS_RAW))

MODE_DEBUG_CFLAGS_RAW := \
  -O0 -g3 -ggdb \
  -fno-omit-frame-pointer -fvar-tracking-assignments -frecord-gcc-switches \
  -fno-inline -fno-inline-functions -fno-inline-functions-called-once \
  -fno-strict-aliasing -DDEBUG
MODE_DEBUG_CFLAGS := $(call cc-options,$(MODE_DEBUG_CFLAGS_RAW))
DBG_PREFIX_MAP := $(call cc-options,-fdebug-prefix-map=$(CURDIR)/$(SRCDIR)=src)

# ---------- Optional LTO (default: on for release builds only) ----------
LTO ?= 1
LTO_CFLAGS := $(call cc-options,-flto -fuse-linker-plugin)

# ---------- Aux-info availability (GCC-only; gated) ----------
HAVE_AUX_INFO := $(shell printf 'int x;\n' | \
  $(CC) -Werror -x c -c -aux-info /dev/null -o /dev/null - >/dev/null 2>&1 && echo yes || echo no)

# ---------- Per-config metadata ----------
HEADLESS_OBJDIR := $(OBJROOT)/headless
HEADLESS_BIN    := $(BINDIR)/chip8-headless
HEADLESS_CPP    := -DHEADLESS
HEADLESS_C      := $(MODE_NORMAL_CFLAGS)
HEADLESS_LD     :=
HEADLESS_LIBS   :=

SDL_OBJDIR := $(OBJROOT)/sdl
SDL_BIN    := $(BINDIR)/chip8
SDL_CPP    := -DUSE_SDL $(SDL_PKG_CFLAGS)
SDL_C      := $(MODE_NORMAL_CFLAGS)
SDL_LD     :=
SDL_LIBS   := $(SDL_PKG_LIBS)

DEBUG_OBJDIR := $(OBJROOT)/debug
DEBUG_BIN    := $(BINDIR)/chip8-debug
DEBUG_CPP    := -DUSE_SDL $(SDL_PKG_CFLAGS)
DEBUG_C      := $(MODE_DEBUG_CFLAGS) $(DBG_PREFIX_MAP)
DEBUG_LD     :=
DEBUG_LIBS   := $(SDL_PKG_LIBS)
DEBUG_AUX    := 1

RELEASE_OBJDIR := $(OBJROOT)/release
RELEASE_BIN    := $(BINDIR)/chip8-release
RELEASE_CPP    := -DUSE_SDL $(SDL_PKG_CFLAGS)
RELEASE_C      := $(MODE_RELEASE_CFLAGS) $(if $(filter 1,$(LTO)),$(LTO_CFLAGS),)
RELEASE_LD     := $(if $(filter 1,$(LTO)),$(LTO_CFLAGS),)
RELEASE_LIBS   := $(SDL_PKG_LIBS)

RELSZ_OBJDIR := $(OBJROOT)/release_size
RELSZ_BIN    := $(BINDIR)/chip8-release-size
RELSZ_CPP    := -DUSE_SDL $(SDL_PKG_CFLAGS)
RELSZ_C      := $(MODE_RELEASE_SIZE_CFLAGS) $(if $(filter 1,$(LTO)),$(LTO_CFLAGS),)
RELSZ_LD     := $(if $(filter 1,$(LTO)),$(LTO_CFLAGS),)
RELSZ_LIBS   := $(SDL_PKG_LIBS)

# ---------- Objects ----------
HEADLESS_OBJS := $(patsubst $(SRCDIR)/%.c,$(HEADLESS_OBJDIR)/%.o,$(SRCS))
SDL_OBJS      := $(patsubst $(SRCDIR)/%.c,$(SDL_OBJDIR)/%.o,$(SRCS))
DEBUG_OBJS    := $(patsubst $(SRCDIR)/%.c,$(DEBUG_OBJDIR)/%.o,$(SRCS))
RELEASE_OBJS  := $(patsubst $(SRCDIR)/%.c,$(RELEASE_OBJDIR)/%.o,$(SRCS))
RELSZ_OBJS    := $(patsubst $(SRCDIR)/%.c,$(RELSZ_OBJDIR)/%.o,$(SRCS))

# ---------- Rule generator ----------
define GEN_CFG_RULES
$($(1)_BIN): $($(1)_OBJS) $(ROM_OBJS)
	$(Q)mkdir -p $$(@D)
	$(Q)echo "Linking $$@..."
	$(Q)$(CC) $(LDFLAGS) $($(1)_LD) $$^ -o $$@ $(LDLIBS) $($(1)_LIBS)

$($(1)_OBJDIR)/%.o: $(SRCDIR)/%.c
	$(Q)mkdir -p $$(@D)
	$(Q)echo "Compiling $$<..."
	$(Q)$(CC) $(CPPFLAGS) $($(1)_CPP) $(CFLAGS) $($(1)_C) \
	  $(if $(and $($(1)_AUX),$(filter yes,$(HAVE_AUX_INFO))),-aux-info $($(1)_OBJDIR)/$$*.aux,) \
	  -c $$< -o $$@
endef

$(eval $(call GEN_CFG_RULES,HEADLESS))
$(eval $(call GEN_CFG_RULES,SDL))
$(eval $(call GEN_CFG_RULES,DEBUG))
$(eval $(call GEN_CFG_RULES,RELEASE))
$(eval $(call GEN_CFG_RULES,RELSZ))

# ---------- Embedded ROMs ----------
$(ROM_MANIFEST): $(ROM_FILES) Makefile
	$(Q)mkdir -p $(@D)
	$(Q){ \
	  echo "/* Auto-generated from $(ROMDIR). Do not edit. */"; \
	  echo "#ifndef ROMS_MANIFEST_H"; \
	  echo "#define ROMS_MANIFEST_H"; \
	  echo "#include <stddef.h>"; \
	  echo "#include <stdint.h>"; \
	  echo ""; \
	  set -- $(ROM_FILES); \
	  count=$$#; \
	  if [ "$$count" -eq 0 ]; then \
	    echo "#define ROMS_EMBEDDED_LIST"; \
	    echo "#define ROMS_EMBEDDED_COUNT 0"; \
	  else \
	    for f in $(ROM_FILES); do \
	      name=$$(basename "$$f"); \
	      sym=$$(printf "%s" "$$name" | sed 's/[^A-Za-z0-9_]/_/g'); \
	      echo "extern const uint8_t _binary_$(ROMDIR)_$${sym}_start[];"; \
	      echo "extern const uint8_t _binary_$(ROMDIR)_$${sym}_end[];"; \
	    done; \
	    echo ""; \
	    echo "#define ROMS_EMBEDDED_LIST \\"; \
	    i=0; \
	    for f in $(ROM_FILES); do \
	      name=$$(basename "$$f"); \
	      sym=$$(printf "%s" "$$name" | sed 's/[^A-Za-z0-9_]/_/g'); \
	      i=$$((i+1)); \
	      if [ "$$i" -lt "$$count" ]; then \
	        echo "  { \"$$name\", \".roms.$$name\", _binary_$(ROMDIR)_$${sym}_start, _binary_$(ROMDIR)_$${sym}_end }, \\"; \
	      else \
	        echo "  { \"$$name\", \".roms.$$name\", _binary_$(ROMDIR)_$${sym}_start, _binary_$(ROMDIR)_$${sym}_end }"; \
	      fi; \
	    done; \
	    echo ""; \
	    echo "#define ROMS_EMBEDDED_COUNT $$count"; \
	  fi; \
	  echo "#endif"; \
	} > $@

$(OBJROOT)/roms/%.o: $(ROMDIR)/%
	$(Q)mkdir -p $(@D)
	$(Q)$(LD) -r -b binary $< -o $@
	$(Q)$(OBJCOPY) --rename-section .data=.roms.$*,alloc,load,readonly,data,contents \
	  --add-section .note.GNU-stack=/dev/null \
	  --set-section-flags .note.GNU-stack=contents,readonly \
	  $@

$(HEADLESS_OBJDIR)/roms_embedded.o: $(ROM_MANIFEST)
$(SDL_OBJDIR)/roms_embedded.o: $(ROM_MANIFEST)
$(DEBUG_OBJDIR)/roms_embedded.o: $(ROM_MANIFEST)
$(RELEASE_OBJDIR)/roms_embedded.o: $(ROM_MANIFEST)
$(RELSZ_OBJDIR)/roms_embedded.o: $(ROM_MANIFEST)

# ---------- Targets ----------
all: sdl
headless: $(HEADLESS_BIN)
sdl:      $(SDL_BIN)
debug:    $(DEBUG_BIN)
release:  $(RELEASE_BIN)
release_size: $(RELSZ_BIN)

format:
	$(Q)clang-format -i $(SRCDIR)/*.c $(HDRDIR)/*.h

clean:
	$(Q)rm -rf $(OBJROOT) $(BINDIR)

rebuild: clean all

# Include deps (all configs)
-include $(wildcard $(OBJROOT)/*/*.d)

.PHONY: all headless sdl debug release release_size clean rebuild format
