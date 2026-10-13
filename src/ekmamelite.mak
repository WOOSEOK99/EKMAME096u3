# uncomment the following lines to include a CPU core
CPUS+=Z80@
CPUS+=M68000@

# uncomment the following lines to include a sound core
SOUNDS+=CUSTOM@
SOUNDS+=AY8910@
SOUNDS+=YM2151@
SOUNDS+=YM2151_ALT@
SOUNDS+=YM2610@
SOUNDS+=YM2610B@
SOUNDS+=YM2612@
SOUNDS+=ADPCM@
SOUNDS+=OKIM6295@
SOUNDS+=QSOUND@

DRVLIBS = \
	$(OBJ)/capcom.a $(OBJ)/neogeo.a \

$(OBJ)/capcom.a: \
	$(OBJ)/vidhrdw/cps1.o $(OBJ)/drivers/cps1.o $(OBJ)/drivers/cps2.o \
	$(OBJ)/machine/kabuki.o \

$(OBJ)/neogeo.a: \
	$(OBJ)/machine/neogeo.o $(OBJ)/machine/pd4990a.o $(OBJ)/machine/neocrypt.o \
	$(OBJ)/vidhrdw/neogeo.o $(OBJ)/drivers/neogeo.o \


COREOBJS += $(OBJ)/litedriver.o $(OBJ)/cheat.o

# generated text files
TEXTS += gamelist.txt

#gamelist.txt: $(EMULATOR)
#	@echo Generating $@...
#	@$(CURPATH)$(EMULATOR) -gamelist -noclones -sortname > docs/gamelist.txt
