########################################################################
############################ USER SETTINGS #############################
########################################################################

TARGET  := chess-engine

CC      :=  clang++
FLAGS   := -Wall -Wextra -flto -ffunction-sections #-fconstexpr-steps=1000000000
STD     := -std=c++17
RELEASE := -Ofast -march=native -m64 -DNDEBUG -ggdb
PROFILE := -Ofast -march=native -g3 -fno-omit-frame-pointer
DEBUG   := -g3 -ggdb -fno-omit-frame-pointer
LIBS    :=

OBJDIR  := obj

PGO     := $(RELEASE) -g -fprofile-instr-generate
PGO_USE := -fprofile-instr-use=$(TARGET).profdata
PGO_ARG := 5

############################################################################
############################## MAKEFILE RULES ##############################
############################################################################

SRC     := $(wildcard *.cpp) $(wildcard */*.cpp)
INC     := $(wildcard *.hpp) $(wildcard */*.hpp)

OBJ     := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC))
DEPS    := $(patsubst %.cpp,$(OBJDIR)/%.d,$(SRC))

release: FLAGS += $(RELEASE) $(PGO_USE)
release: all

profile: FLAGS += $(PROFILE) $(PGO_USE)
profile: all

debug:   FLAGS += $(DEBUG)
debug:   all

pgo: FLAGS += $(PGO)
pgo: clean all
	@$(ECHO) "$(BLU)STARTED  $(GRN)PROFILING$(RST)"
	-@rm -f $(TARGET).profraw $(TARGET).profdata
	@LLVM_PROFILE_FILE=$(TARGET).profraw ./$(TARGET) $(PGO_ARG) 1>/dev/null
	@$(ECHO) $(FINISHED) "$(GRN)PROFILING$(RST)\n"
	@llvm-profdata merge -output=$(TARGET).profdata $(TARGET).profraw
	@make -s clean
	@make -s release
	@$(ECHO) $(FINISHED) "$(GRN)PGO$(RST)\n"

all: build $(TARGET)
	@$(ECHO) $(FINISHED) "$(GRN)COMPILING $(RST)\n"

$(OBJDIR)/%.o: %.cpp Makefile
	@mkdir -p $(@D)
	@$(ECHO) $(BUILDING) "$(BLU)$(patsubst %.cpp,%.o,$<)$(RST) \
	-> $(BLU)$(patsubst %.cpp,%.d,$<)$(RST)"
	@$(CC) $(FLAGS) $(STD) -MMD -MP -c $< -o $@

$(TARGET): $(OBJ)
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) $(STD) $(LIBS) $^ -o $(TARGET)
	@$(ECHO) $(BUILDING) $(TARGET)

-include $(DEPS)

build:
	@$(STARTING) && sleep 0.2
	@mkdir -p $(OBJDIR)

clean:
	@$(STARTING) && sleep 0.2
	-@rm -rf $(OBJDIR) $(TARGET)
	@$(ECHO) $(DELETING) "$(BLU)object files$(RST)"     && sleep 0.2
	@$(ECHO) $(DELETING) "$(BLU)dependency files$(RST)" && sleep 0.2
	@$(ECHO) $(DELETING) "$(BLU)$(OBJDIR)/$(RST)"       && sleep 0.2
	@$(ECHO) $(DELETING) "$(BLU)${TARGET}$(RST)"        && sleep 0.2
	@$(ECHO) $(FINISHED) "$(GRN)CLEANING $(RST)\n"

info:
	@echo -e "$(GRN)GENERAL:\n\
	$(YLW)   TARGET  | $(BLU)$(TARGET)$(RST)\n\
	$(YLW)   CC      | $(BLU)$(CC)$(RST)\n\
	$(YLW)   LIBS    | $(BLU)$(LIBS)$(RST)\n\
	$(YLW)   FLAGS   | $(BLU)$(FLAGS)$(RST)\n\
	$(YLW)   STD     | $(BLU)$(STD)$(RST)\n\
	$(YLW)   RELEASE | $(BLU)$(RELEASE)$(RST)\n\
	$(YLW)   DEBUG   | $(BLU)$(DEBUG)$(RST)\n "

	@echo -e "$(GRN)SOURCES:$(BLU)\n $(patsubst %.cpp,  %.cpp\n,$(SRC))"
	@echo -e "$(GRN)INCLUDE:$(BLU)\n $(patsubst %.hpp,  %.hpp\n,$(INC))"
	@echo -e "$(GRN)OBJECTS:$(BLU)\n $(patsubst %.cpp,  %.o\n,$(SRC))"
	@echo -e "$(GRN)DEPENDS:$(BLU)\n $(patsubst %.cpp,  %.d\n,$(SRC))"

.PHONY: all build debug release profile pgo clean info

############################################################################
######################### PROGRESS INDICATION TOOLS ########################
############################################################################

RED := \033[31m
GRN := \033[32m
YLW := \033[33m
BLU := \033[34m
RST := \033[0m
CLR := \033[2K

STARTING  := echo -en "\r$(YLW)  0% $(BLU)STARTING $(RST)"
CREATING  := "$(GRN)CREATING$(RST)"
BUILDING  := "$(GRN)BUILDING$(RST)"
DELETING  := "$(RED)DELETING$(RST)"
FINISHED  := "$(BLU)FINISHED$(RST)"

ifndef ECHO
TOTAL  != $(MAKE) $(MAKECMDGOALS) --dry-run ECHO=FOUND | grep -c FOUND
CURRENT = $(eval HIT_N != expr $(HIT_N) + 1)$(HIT_N)
PERCENT = expr $(CURRENT) '*' 100 / $(TOTAL)
ECHO    = echo -en "\r$(CLR)$(YLW)`expr " \`$(PERCENT)\`" \
                                 : '.*\(...\)$$'`%$(RST)"
endif

############################################################################
############################################################################
############################################################################
