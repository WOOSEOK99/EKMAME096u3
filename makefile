include preconfig.def

# select compiler
# USE_GCC = 1
# USE_VC = 1
# INTEL = 1
# if compiler is not selected, GCC is used as the default.

ifndef USE_VC
    ifndef USE_GCC
        USE_GCC = 1
    endif
endif

ifneq ($(NO_DLL),)
    ifneq ($(WINUI),)
        SUFFIX = 32nodll
    else
        SUFFIX = nodll
    endif
# no dll version
    DONT_USE_DLL=1
else
# always define WINUI = 1 for mamelib.dll
    WINUI=1
endif

ifdef USE_VC
# uncomment one of the next lines to use Whole Program Optimization
# USE_IPO = 1
endif

# uncomment one of the next lines to build a target-optimized build
# ATHLON = 1
# I686 = 1
# P4 = 1
# PM = 1

# uncomment next line to include the symbols for symify
# SYMBOLS = 1

# uncomment next line to generate a link map for exception handling in windows
# MAP = 1

# uncomment next line to include the debugger
# DEBUG = 1

# uncomment next line to use the new multiwindow debugger
NEW_DEBUGGER = 1

# uncomment next line to use Assembler 68000 engine
# X86_ASM_68000 = 1

# uncomment next line to use Assembler 68020 engine
# X86_ASM_68020 = 1

# uncomment next line to use DRC 68K engine
# X86_M68K_DRC = 1

# uncomment next line to use DRC MIPS3 engine
X86_MIPS3_DRC = 1

# uncomment next line to use DRC PowerPC engine
X86_PPC_DRC = 1

# uncomment next line to use cygwin compiler
# COMPILESYSTEM_CYGWIN	= 1

# uncomment next line to build expat as part of MAME build
BUILD_EXPAT = 1

# uncomment next line to build zlib as part of MAME build
BUILD_ZLIB = 1


# set this the operating system you're building for
# MAMEOS = msdos
# MAMEOS = windows
ifeq ($(MAMEOS),)
MAMEOS = windows
endif

# extension for executables
EXE = .exe

# CPU core include paths
VPATH=src $(wildcard src/cpu/*)

# compiler, linker and utilities
ifdef USE_GCC
    ifndef USE_XGCC
        AR = @ar
        CC = @gcc
        XCC = @gcc
        LD = @gcc
        DLLWRAP = @dllwrap
    else
        AR = @i686-pc-mingw32-ar
        CC = @i686-pc-mingw32-gcc
        XCC = @gcc
        LD = @i686-pc-mingw32-gcc
        DLLWRAP = @i686-pc-mingw32-dllwrap
    endif
else
    ifdef INTEL
        AR = @xilib
        CC = @icl
        LD = @xilink
    else
        AR = @lib
        CC = @cl
        LD = @link
    endif
endif

ASM = @nasm
ASMFLAGS = -f coff
MD = -mkdir.exe
RM = @rm -f

ifdef USE_GCC
    WINDOWS_PROGRAM = -mwindows
    CONSOLE_PROGRAM = -mconsole
else
    WINDOWS_PROGRAM = -subsystem:windows
    CONSOLE_PROGRAM = -subsystem:console
endif

ifdef I686
    P6OPT = ppro
else
    P6OPT = notppro
endif

ifeq ($(MAMEOS),msdos)
    PREFIX = d
else
    PREFIX =
endif

ifdef USE_GCC
    XEXTRA_SUFFIX = $(EXTRA_SUFFIX)

    # by default, compile for Pentium target and add no suffix
    ARCHSUFFIX =
    ARCH = -march=pentium

    ifdef ATHLON
        ARCHSUFFIX = at
        ARCH = -march=athlon -m3dnow
    endif

    ifdef ATHLONXP
        ARCHSUFFIX = ax
        ARCH = -march=athlon-xp -m3dnow -msse
    endif

    ifdef I686
        ARCHSUFFIX = pp
        ARCH = -march=i686 -mmmx
    endif

    ifdef P4
        ARCHSUFFIX = p4
        ARCH = -march=pentium4 -msse2
    endif

    ifdef PM
        ARCHSUFFIX = pm
        ARCH = -march=pentium3 -msse2
    endif
else
    XEXTRA_SUFFIX = i$(EXTRA_SUFFIX)

    # by default, compile for Pentium target and add no suffix
    ARCHSUFFIX =
    ARCH = -G5

    ifdef I686
        ARCHSUFFIX = pp
        ARCH = -G6
    endif

    ifdef P4
        ARCHSUFFIX = p4
        ARCH = -G7 -QxN -arch:SSE2
    endif

    ifdef PM
        ARCHSUFFIX = pm
        ARCH = -G6 -QxB -arch:SSE2
    endif
endif

NAME = $(PREFIX)$(TARGET)$(SUFFIX)$(ARCHSUFFIX)$(XEXTRA_SUFFIX)
ifeq ($(NO_DLL),)
    GUINAME = $(PREFIX)$(TARGET)32$(SUFFIX)$(ARCHSUFFIX)$(XEXTRA_SUFFIX)
endif

# debug builds just get the 'd' suffix and nothing more
ifdef DEBUG
    NAME = $(PREFIX)$(TARGET)$(SUFFIX)$(XEXTRA_SUFFIX)d
    ifeq ($(NO_DLL),)
        GUINAME = $(PREFIX)$(TARGET)32$(SUFFIX)$(XEXTRA_SUFFIX)d
    endif
endif

# build the targets in different object dirs, since mess changes
# some structures and thus they can't be linked against each other.
OBJ = obj/$(NAME)
BIN = bin

ifneq ($(NO_DLL),)
    EMULATOR = $(BIN)/$(EXENAME)$(EXE)
else
    EMULATORLIB = $(NAME)lib
    EMULATORDLL = $(EMULATORLIB).dll
    EMULATORCLI = $(NAME)$(EXE)
    EMULATORGUI = $(GUINAME)$(EXE)
    EMULATOR    = $(EMULATORDLL) $(EMULATORCLI) $(EMULATORGUI)
endif

ifdef USE_GCC
    DEFS = -DX86_ASM -DLSB_FIRST -DINLINE="static __inline__" -Dasm=__asm__ -DCRLF=3 -DXML_STATIC
else
    DEFS = -DLSB_FIRST=1 -DINLINE='static __forceinline' -Dinline=__inline -D__inline__=__inline -DCRLF=3 -DXML_STATIC
endif

include config.def

ifdef NEW_DEBUGGER
DEFS += -DNEW_DEBUGGER
endif

ifneq ($(LAMER),)
    DEFS += -DLAMER
endif

ifdef X86_M68K_DRC
    DEFS += -DX86_M68K_DRC
endif

ifdef USE_GCC
    CFLAGS = -std=gnu99 -Isrc -Isrc/includes -Isrc/zlib -Iextra/include -Isrc/debug -Isrc/$(MAMEOS) -I$(OBJ)/cpu/m68000 -Isrc/cpu/m68000 \

    ifeq ($(USE_4X_EFFECT),1)
		CFLAGS += -fno-gcse
    endif

    ifneq ($(W_ERROR),)
        CFLAGS += -Werror 
    else
        CFLAGS += -Wno-error 
    endif

    ifneq ($(SYMBOLS),)
        CFLAGS += -O0 -Wall -Wno-unused -g
    else
        CFLAGS += -DNDEBUG \
			$(ARCH) -O3 -fomit-frame-pointer -fno-strict-aliasing \
			-Wall -Wno-sign-compare -Wunused \
			-Wpointer-arith -Wbad-function-cast -Wcast-align \
			-Wstrict-prototypes -Wundef \
			-Wwrite-strings \
			-Wdisabled-optimization \
			-Wno-unused-function \
		#	-Wno-strict-aliasing \
		#	-Wformat-security 
		#	-Wredundant-decls
		#	-Wfloat-equal
		#	-Wunreachable-code -Wpadded
		#	-W had to remove because of the "missing initializer" warning
		#	-Wlarger-than-262144  \
		#	-Wcast-qual \
		#	-Wconversion \
		#	-Wmissing-prototypes \
		#	-Wmissing-declarations
    endif

    ifdef I686
    # If you have a trouble in I686 build, try to remove a comment.
    #    CFLAGS += -fno-builtin -fno-omit-frame-pointer 
    endif

    # extra options needed *only* for the osd files
    CFLAGSOSDEPEND = $(CFLAGS)

    # the windows osd code at least cannot be compiled with -pedantic
    CFLAGSPEDANTIC = $(CFLAGS) -pedantic
else
    CFLAGS = -Isrc -Isrc/includes -Isrc/zlib -Isrc/debug -Isrc/$(MAMEOS) -I$(OBJ)/cpu/m68000 -Isrc/cpu/m68000 \
             -W3 -nologo

    ifdef INTEL
        CFLAGS += -Qc99
    endif

    ifneq ($(W_ERROR),)
        CFLAGS += -WX
    endif

    ifneq ($(SYMBOLS),)
        CFLAGS += -Od -RTC1 -MLd -ZI -Zi -GS
    else
        ifneq ($(USE_IPO),)
            ifdef INTEL
                CFLAGS += -Qipo -Qipo_obj
            else
                CFLAGS += -GL
            endif
        endif

        ifdef INTEL
            CFLAGS += -O3 -Qip -Qvec_report0
        else
            CFLAGS += -O2
        endif

        CFLAGS += -Og -Ob2 -Oi -Ot -Oy -GA -Gy -GF
        CFLAGS += -DNDEBUG -ML $(ARCH)
    endif
endif

ifdef USE_GCC
    LDFLAGS = -Lextra/lib

    ifeq ($(SYMBOLS),)
        #LDFLAGS = -s -Wl,--warn-common
        LDFLAGS += -s
    endif

    ifneq ($(MAP),)
        MAPFLAGS = -Wl,-Map,$(NAME).map
        MAPDLLFLAGS = -Wl,-Map,$(EMULATORLIB).map
        MAPCLIFLAGS = -Wl,-Map,$(NAME).map
        MAPGUIFLAGS = -Wl,-Map,$(GUINAME).map
    else
        MAPFLAGS =
        MAPDLLFLAGS =
        MAPCLIFLAGS =
        MAPGUIFLAGS =
    endif
else
    ARFLAGS = -nologo

    LDFLAGS += -machine:x86 -nologo -opt:noref

    ifneq ($(SYMBOLS),)
        LDFLAGS += -debug:full -incremental -nodefaultlib:libc
    else
        LDFLAGS += -release -incremental:no

        ifneq ($(USE_IPO),)
            ifndef INTEL
                ARFLAGS += -LTCG
                LDFLAGS += -LTCG
            endif
        endif
    endif

    ifneq ($(MAP),)
        MAPFLAGS = -map
        MAPDLLFLAGS = -map
        MAPCLIFLAGS = -map
        MAPGUIFLAGS = -map
    else
        MAPFLAGS =
        MAPDLLFLAGS =
        MAPCLIFLAGS =
        MAPGUIFLAGS =
    endif
endif

OBJDIRS = $(BIN) obj $(OBJ) $(OBJ)/cpu $(OBJ)/sound $(OBJ)/$(MAMEOS) \
	$(OBJ)/drivers $(OBJ)/machine $(OBJ)/vidhrdw $(OBJ)/sndhrdw $(OBJ)/debug
ifdef MESS
OBJDIRS += $(OBJ)/mess $(OBJ)/mess/systems $(OBJ)/mess/machine \
	$(OBJ)/mess/vidhrdw $(OBJ)/mess/sndhrdw $(OBJ)/mess/tools
endif

# start with an empty set of libs
LIBS = 

all:	maketree emulator extrafiles

# include the various .mak files
include src/core.mak
include src/$(TARGET).mak
include src/rules.mak
include src/$(MAMEOS)/$(MAMEOS).mak

ifdef BUILD_EXPAT
CFLAGS += -Isrc/expat
OBJDIRS += $(OBJ)/expat
EXPAT = $(OBJ)/libexpat.a
COREOBJS += $(EXPAT)
else
LIBS += -lexpat
EXPAT =
endif

ifdef BUILD_ZLIB
CFLAGS += -Isrc/zlib
OBJDIRS += $(OBJ)/zlib
ZLIB = $(OBJ)/libz.a
COREOBJS += $(ZLIB)
else
LIBS += -lz
ZLIB =
endif

ifdef DEBUG
DBGDEFS = -DMAME_DEBUG
else
DBGDEFS =
DBGOBJS =
endif

ifdef COMPILESYSTEM_CYGWIN
CFLAGS	+= -mno-cygwin
LDFLAGS	+= -mno-cygwin
endif

emulator:	maketree $(EMULATOR)

#extrafiles:	$(TOOLS) $(TEXTS)
extrafiles:	$(TOOLS)

# combine the various definitions to one
CDEFS = $(DEFS) $(COREDEFS) $(CPUDEFS) $(SOUNDDEFS) $(ASMDEFS) $(DBGDEFS)

ifneq ($(NO_DLL),)
# do not use dllimport
    CDEFS += -DDONT_USE_DLL

    ifneq ($(WINUI),)
        OSOBJS += $(GUIOBJS)
        LIBS += $(GUILIBS)
    else
        OSOBJS += $(CLIOBJS)
        LIBS += $(CLILIBS)
    endif
endif

# primary target
ifneq ($(NO_DLL),)
    $(EMULATOR): $(OBJS) $(COREOBJS) $(OSOBJS) $(DRVLIBS)
else
    $(EMULATORDLL): $(OBJS) $(COREOBJS) $(OSOBJS) $(DRVLIBS)
endif

# always recompile the version string
ifdef USE_GCC
	$(CC) $(CDEFS) $(CFLAGSPEDANTIC) -c src/version.c -o $(OBJ)/version.o
else
	@echo -n Compiling\040
	$(CC) $(CDEFS) $(CFLAGS) -c src/version.c -Fo$(OBJ)/version.o
endif
	@echo Linking $@...

ifneq ($(NO_DLL),)

    ifdef USE_GCC
        ifneq ($(WINUI),)
			$(LD) $(LDFLAGS) $(WINDOWS_PROGRAM) $(OBJS) $(COREOBJS) $(OSOBJS) $(LIBS) $(DRVLIBS) -o $@ $(MAPFLAGS)
        else
			$(LD) $(LDFLAGS) $(CONSOLE_PROGRAM) $(OBJS) $(COREOBJS) $(OSOBJS) $(LIBS) $(DRVLIBS) -o $@ $(MAPFLAGS)
        endif
    else
        ifneq ($(WINUI),)
			$(LD) $(LDFLAGS) $(WINDOWS_PROGRAM) $(OBJS) $(COREOBJS) $(OSOBJS) $(LIBS) $(DRVLIBS) -out:$(EMULATOR) $(MAPFLAGS)
        else
			$(LD) $(LDFLAGS) $(CONSOLE_PROGRAM) $(OBJS) $(COREOBJS) $(OSOBJS) $(LIBS) $(DRVLIBS) -out:$(EMULATOR) $(MAPFLAGS)
        endif
    endif

    ifneq ($(USE_UPX),)
		upx $(EMULATOR)
    endif

else

	$(RM) $@
    ifdef USE_GCC
		$(DLLWRAP) --dllname=$@ --driver-name=gcc \
			$(LDFLAGS) $(OBJS) $(COREOBJS) $(OSOBJS) $(LIBS) $(DRVLIBS) $(MAPDLLFLAGS)
    else
		$(LD) $(LDFLAGS) -dll -out:$@ $(OBJS) $(COREOBJS) $(OSOBJS) $(LIBS) $(DRVLIBS) $(MAPDLLFLAGS)
    endif
    ifneq ($(USE_UPX),)
		upx -9 $@
    endif

# gui target
    $(EMULATORGUI): $(EMULATORDLL) $(GUIOBJS)
		@echo Linking $@...
    ifdef USE_GCC
		$(LD) $(LDFLAGS) $(WINDOWS_PROGRAM) $^ -o $@ $(GUILIBS) $(MAPGUIFLAGS)
    else
		$(LD) $(LDFLAGS) $(WINDOWS_PROGRAM) $(EMULATORLIB).lib $(GUIOBJS) -out:$@ $(GUILIBS) $(LIBS) $(MAPGUIFLAGS)
    endif
    ifneq ($(USE_UPX),)
		upx -9 $@
    endif

# cli target
    $(EMULATORCLI):	$(EMULATORDLL) $(CLIOBJS)
		@echo Linking $@...
    ifdef USE_GCC
		$(LD) $(LDFLAGS) $(CONSOLE_PROGRAM) $^ -o $@ $(CLILIBS) $(MAPCLIFLAGS)
    else
		$(LD) $(LDFLAGS) $(CONSOLE_PROGRAM) $(EMULATORLIB).lib $(CLIOBJS) -out:$@ $(CLILIBS) $(MAPCLIFLAGS)
    endif
    ifneq ($(USE_UPX),)
		upx -9 $@
    endif

endif

romcmp$(EXE): $(OBJ)/romcmp.o $(OBJ)/unzip.o $(OBJ)/ui_lang.o $(VCOBJS) $(ZLIB)
	@echo Linking $@...
    ifdef USE_GCC
	$(LD) $(LDFLAGS) $(CONSOLE_PROGRAM) $^ $(LIBS) -o $@
    else
	$(LD) $(LDFLAGS) $(CONSOLE_PROGRAM) $^ $(LIBS) -out:$@
    endif

chdman$(EXE): $(OBJ)/chdman.o $(OBJ)/chd.o $(OBJ)/chdcd.o $(OBJ)/cdrom.o $(OBJ)/md5.o $(OBJ)/sha1.o $(OBJ)/version.o $(ZLIB)
	@echo Linking $@...
    ifdef USE_GCC
	$(LD) $(LDFLAGS) $(CONSOLE_PROGRAM) $^ $(LIBS) $(ZLIB) -o $@
    else
	$(LD) $(LDFLAGS) $(CONSOLE_PROGRAM) $^ $(LIBS) $(ZLIB) -out:$@
    endif

xml2info$(EXE): $(OBJ)/xml2info.o $(EXPAT)
	@echo Linking $@...
    ifdef USE_GCC
	$(LD) $(LDFLAGS) $(CONSOLE_PROGRAM) $^ $(LIBS) $(ZLIB) -o $@
    else
	$(LD) $(LDFLAGS) $(CONSOLE_PROGRAM) $^ $(LIBS) $(ZLIB) -out:$@
    endif

# secondary libraries
$(OBJ)/libexpat.a: $(OBJ)/expat/xmlparse.o $(OBJ)/expat/xmlrole.o $(OBJ)/expat/xmltok.o

$(OBJ)/libz.a: $(OBJ)/zlib/adler32.o $(OBJ)/zlib/compress.o $(OBJ)/zlib/crc32.o $(OBJ)/zlib/deflate.o \
				$(OBJ)/zlib/gzio.o $(OBJ)/zlib/inffast.o $(OBJ)/zlib/inflate.o $(OBJ)/zlib/infback.o \
				$(OBJ)/zlib/inftrees.o $(OBJ)/zlib/trees.o $(OBJ)/zlib/uncompr.o $(OBJ)/zlib/zutil.o

$(OBJ)/$(MAMEOS)/%.o: src/$(MAMEOS)/%.c
ifdef USE_GCC
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGSOSDEPEND) -c $< -o $@
else
	@echo -n Compiling\040
	$(CC) $(CDEFS) $(CFLAGS) -Fo$@ -c $<
endif

$(OBJ)/%.o: src/%.c
ifdef USE_GCC
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@
else
	@echo -n Compiling\040
	$(CC) $(CDEFS) $(CFLAGS) -Fo$@ -c $<
endif

# compile generated C files for the 68000 emulator
$(M68000_GENERATED_OBJS): $(OBJ)/cpu/m68000/m68kmake$(EXE)
	@echo Compiling $(subst .o,.c,$@)...
ifdef USE_GCC
	$(CC) $(CDEFS) $(CFLAGSPEDANTIC) -c $*.c -o $@
else
	$(CC) $(CDEFS) $(CFLAGS) -Fo$@ -c $*.c
endif

# additional rule, because m68kcpu.c includes the generated m68kops.h :-/
$(OBJ)/cpu/m68000/m68kcpu.o: $(OBJ)/cpu/m68000/m68kmake$(EXE)

# generate C source files for the 68000 emulator
$(OBJ)/cpu/m68000/m68kmake$(EXE): src/cpu/m68000/m68kmake.c src/cpu/m68000/m68k_in.c
	@echo M68K make $<...
ifdef USE_GCC
	$(XCC) $(CDEFS) $(CFLAGSPEDANTIC) $(CONSOLE_PROGRAM) -DDOS -o $(OBJ)/cpu/m68000/m68kmake$(EXE) $<
else
	$(CC) $(CDEFS) $(CFLAGS) -Fe$@ -Fo$(OBJ)/cpu/m68000 $< -link $(CONSOLE_PROGRAM)
endif
	@echo Generating M68K source files...
	$(OBJ)/cpu/m68000/m68kmake$(EXE) $(OBJ)/cpu/m68000 src/cpu/m68000/m68k_in.c

# compile generated C files for the 68000 emulator
$(M68000DRC_GENERATED_OBJS): $(OBJ)/cpu/m68000/d68kmake$(EXE)
	@echo Compiling $(subst .o,.c,$@)...
ifdef USE_GCC
	$(CC) $(CDEFS) $(CFLAGSPEDANTIC) -c $*.c -o $@
else
	$(CC) $(CDEFS) $(CFLAGS) -Fo$@ -c $*.c
endif

# additional rule, because d68kcpu.c includes the generated d68kops.h :-/
$(OBJ)/cpu/m68000/d68kcpu.o: $(OBJ)/cpu/m68000/d68kmake$(EXE)

# generate C source files for the 68000 emulator
$(OBJ)/cpu/m68000/d68kmake$(EXE): src/cpu/m68000/d68kmake.c src/cpu/m68000/d68k_in.c
	@echo M68K make $<...
ifdef USE_GCC
	$(XCC) $(CDEFS) $(CFLAGSPEDANTIC) $(CONSOLE_PROGRAM) -DDOS -o $(OBJ)/cpu/m68000/d68kmake$(EXE) $<
else
	$(CC) $(CDEFS) $(CFLAGS) -Fe$@ -Fo$(OBJ)/cpu/m68000 $< -link $(CONSOLE_PROGRAM)
endif
	@echo Generating M68K source files...
	$(OBJ)/cpu/m68000/d68kmake$(EXE) $(OBJ)/cpu/m68000 src/cpu/m68000/d68k_in.c

# generate asm source files for the 68000/68020 emulators
$(OBJ)/cpu/m68000/68000.asm:  src/cpu/m68000/make68k.c
	@echo Compiling $<...
ifdef USE_GCC
	$(XCC) $(CDEFS) $(CFLAGSPEDANTIC) $(CONSOLE_PROGRAM) -O0 -DDOS -o $(OBJ)/cpu/m68000/make68k$(EXE) $<
else
	$(CC) $(CDEFS) $(CFLAGS) -Fe$(OBJ)/cpu/m68000/make68k$(EXE) -Fo$(OBJ)/cpu/m68000 $< -link $(CONSOLE_PROGRAM)
endif
	@echo Generating $@...
	@$(OBJ)/cpu/m68000/make68k$(EXE) $@ $(OBJ)/cpu/m68000/68000tab.asm 00 $(P6OPT)

$(OBJ)/cpu/m68000/68010.asm:  src/cpu/m68000/make68k.c
	@echo Compiling $<...
ifdef USE_GCC
	$(XCC) $(CDEFS) $(CFLAGSPEDANTIC) $(CONSOLE_PROGRAM) -O0 -DDOS -o $(OBJ)/cpu/m68000/make68k$(EXE) $<
else
	$(CC) $(CDEFS) $(CFLAGS) -Fe$(OBJ)/cpu/m68000/make68k$(EXE) -Fo$(OBJ)/cpu/m68000 $< -link $(CONSOLE_PROGRAM)
endif
	@echo Generating $@...
	@$(OBJ)/cpu/m68000/make68k$(EXE) $@ $(OBJ)/cpu/m68000/68010tab.asm 10 $(P6OPT)

$(OBJ)/cpu/m68000/68020.asm:  src/cpu/m68000/make68k.c
	@echo Compiling $<...
ifdef USE_GCC
	$(XCC) $(CDEFS) $(CFLAGSPEDANTIC) $(CONSOLE_PROGRAM) -O0 -DDOS -o $(OBJ)/cpu/m68000/make68k$(EXE) $<
else
	$(CC) $(CDEFS) $(CFLAGS) -Fe$(OBJ)/cpu/m68000/make68k$(EXE) -Fo$(OBJ)/cpu/m68000 $< -link $(CONSOLE_PROGRAM)
endif
	@echo Generating $@...
	@$(OBJ)/cpu/m68000/make68k$(EXE) $@ $(OBJ)/cpu/m68000/68020tab.asm 20 $(P6OPT)

# generated asm files for the 68000 emulator
$(OBJ)/cpu/m68000/68000.o:  $(OBJ)/cpu/m68000/68000.asm
	@echo Assembling $<...
	$(ASM) -o $@ $(ASMFLAGS) $(subst -D,-d,$(ASMDEFS)) $<

$(OBJ)/cpu/m68000/68010.o:  $(OBJ)/cpu/m68000/68010.asm
	@echo Assembling $<...
	$(ASM) -o $@ $(ASMFLAGS) $(subst -D,-d,$(ASMDEFS)) $<

$(OBJ)/cpu/m68000/68020.o:  $(OBJ)/cpu/m68000/68020.asm
	@echo Assembling $<...
	$(ASM) -o $@ $(ASMFLAGS) $(subst -D,-d,$(ASMDEFS)) $<

$(OBJ)/%.a:
	@echo Archiving $@...
	$(RM) $@
ifdef USE_GCC
	$(AR) cr $@ $^
else
	$(AR) $(ARFLAGS) -out:$@ $^
endif

$(sort $(OBJDIRS)):
	$(MD) $@

maketree: $(sort $(OBJDIRS))

clean:
	@echo Deleting object tree $(OBJ)...
	$(RM) -r $(OBJ)
	@echo Deleting $(EMULATOR)...
	$(RM) $(EMULATOR)

check: $(EMULATOR) xml2info$(EXE)
	./$(EMULATOR) -listxml > $(NAME).xml
	./xml2info < $(NAME).xml > $(NAME).lst
	./xmllint --valid --noout $(NAME).xml
