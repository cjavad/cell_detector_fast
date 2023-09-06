# remove .exe if on linux or compiling for linux
include compiler.mk
EXECUTABLE	= cells.exe
ARGS		= 

# build directories
INCDIR		= include
SRCDIR		= src
BINDIR		= # platform dependent
DEPDIR		= dep
OBJDIR		= build
RESDIR		= res

PPFLAGS = -D_CRT_SECURE_NO_DEPRECATE
IFLAGS = -I$(INCDIR) -I$(SRCDIR)
CFLAGS = -msse4.2 -mavx2 -mfma -std=c99 -ffp-contract=fast
LFLAGS =

#platform dependents
ifneq ($(patsubst %.exe, %, $(CC)), $(CC))
	WINDOWS = true
endif

ifdef WINDOWS
	CWD:=$(shell cmd.exe /c "echo %cd:\=/%")
	PPFLAGS +=
	LFLAGS += -L$(CWD)/lib/win -lSynchronization -lKernel32 
	BINDIR = dist/win
else
	CWD:=$(shell pwd)
	PPFLAGS += -D_POSIX_C_SOURCE=199309L -D_GNU_SOURCE
	LFLAGS += -L$(CWD)/lib/linux -lpthread -lrt -ldl -lm -Wl,-rpath,.
	BINDIR = dist/linux
endif

#back to nice
DEPDIR := $(BINDIR)/$(DEPDIR)
OBJDIR := $(BINDIR)/$(OBJDIR)

C_FILES := $(shell find $(SRCDIR) -type f -name "*.c")
D_FILES := $(patsubst $(SRCDIR)/%.c, $(DEPDIR)/%.d, $(C_FILES))
O_FILES := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(C_FILES))

IR_FILES := $(shell find $(RESDIR) -type f -name "*")
OR_FILES := $(patsubst $(SRCDIR)/%, $(BINDIR)/$(RESDIR)/%, $(IR_FILES))

include $(D_FILES)

.PHONY: raw build release compile clean purge run lines log deps

raw: CFLAGS +=
raw: PPFLAGS += -DDEBUG
raw: clear_build_log compile

build: CFLAGS += -O2 -Wall -Wextra
build: PPFLAGS += -DDEBUG
build: clear_build_log compile

release: CFLAGS += -O3 -Wall -Wextra
release: PPFLAGS += -DRELEASE
release: clear_build_log clean compile

res: $(BINDIR)/$(OR_FILES)

compile: $(O_FILES) $(D_FILES)
	@echo "!========< LINK >========!"
	@echo $(CC) $(O_FILES) -o $(BINDIR)/$(EXECUTABLE) $(LFLAGS)
	@$(CC) $(O_FILES) -o $(BINDIR)/$(EXECUTABLE) $(LFLAGS) 1>> build.log 2>> build.log
	@echo "!========< $(shell echo Build at $(shell date +'%Y-%m-%d-%T')) >========!"
	@cat build.log

log:
	@cat build.log
	
purge: clean
	@rm -rf $(DEPDIR)

clean:
	@rm -rf $(BINDIR)/$(EXECUTABLE)
	@rm -rf $(OBJDIR)	

clear_build_log:
	@:>build.log
	
clean_deps:
	@rm -rf $(DEPDIR)	

deps: clean_deps $(D_FILES)
		
run: res
	@echo "!========< RUN >========!"
	@rm -f $(BINDIR)/logs/*.log
	@cd $(BINDIR) && ./$(EXECUTABLE) $(ARGS)
	
lines:
#	globbing is beautiful
	@find $(SRCDIR) -type f -name "*.[chi]" | xargs wc -l | sort -n

#build dependency files
$(DEPDIR)/%.d: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	@echo -n "$@ " > $@
	@$(CC) $(IFLAGS) -MM -MT $(patsubst $(DEPDIR)/%.d, $(OBJDIR)/%.o, $@) $(patsubst $(DEPDIR)/%.d, $(SRCDIR)/%.c, $@) >> $@ || rm $@
	@echo $(CC) $(IFLAGS) -MM -MT $(patsubst $(DEPDIR)/%.d, $(OBJDIR)/%.o, $@) $(patsubst $(DEPDIR)/%.d, $(SRCDIR)/%.c, $@)
# 	i hate this, clang on windows insists on not using / in filepaths
	@if [ $(WINDOWS) ]; then \
		sed -i "s/\\\\/\\//g" $@;\
		sed -i "s/[[:space:]]\\// \\\\/g" $@;\
	fi

#build object files
$(OBJDIR)/%.o:
	@mkdir -p $(@D)
	@echo $(CC) $(IFLAGS) $(CFLAGS) $(PPFLAGS) -c $(patsubst $(OBJDIR)/%.o, $(SRCDIR)/%.c, $@) -o $@
	@$(CC) $(IFLAGS) $(CFLAGS) $(PPFLAGS) -c $(patsubst $(OBJDIR)/%.o, $(SRCDIR)/%.c, $@) -o $@ 1>> build.log 2>> build.log

$(BINDIR)/$(RESDIR)/%: $(RESDIR)/%
	@mkdir -p $(BINDIR)/$(RESDIR)
	@cp -l -T $^ $@ 