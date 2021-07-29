######################################################################
###################### PROGRESS INDICATION TOOLS #####################
######################################################################

RED = \033[31m
GRN = \033[32m
YLW = \033[33m
BLU = \033[34m
RST = \033[0m
CLR = \033[2K

STARTING  = echo -ne "\r $(YLW)  0% $(BLU)STARTING $(RST)"
CREATING  = "$(GRN)CREATING$(RST)"
BUILDING  = "$(GRN)BUILDING$(RST)"
DELETING  = "$(RED)DELETING$(RST)"
FINISHED  = "$(BLU)FINISHED$(RST)"

ifndef ECHO
TOTAL  != $(MAKE) $(MAKECMDGOALS) --dry-run ECHO="HIT" | grep -c "HIT"
CURRENT = $(eval HIT_N != expr $(HIT_N) + 1)$(HIT_N)
PERCENT = expr $(CURRENT) '*' 100 / $(TOTAL)
ECHO = echo -ne "\r$(CLR)$(YLW)`expr " \`$(PERCENT)\`" \
                : '.*\(...\)$$'`%$(RST)"
endif

######################################################################
########################### MAKEFILE RULES ###########################
######################################################################

TARGET  := chess-engine

CC      := g++
FLAGS   := -Wall -Wextra -fconstexpr-ops-limit=1000000000
RELEASE := -O3 -march=native
DEBUG   := -g3 -fsanitize=address,undefined
LIBS    :=

SRC     := $(wildcard *.cpp) $(wildcard */*.cpp)
INC     := $(wildcard *.hpp) $(wildcard */*.hpp)

OBJDIR  := obj
OBJ     := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC))
DEPS    := $(patsubst %.cpp,$(OBJDIR)/%.d,$(SRC))

all: build $(TARGET)
	@$(ECHO) $(FINISHED) "$(GRN)COMPILING $(RST)\n"

debug:   FLAGS += $(DEBUG)
debug:   all

release: FLAGS += $(RELEASE)
release: all

$(OBJDIR)/%.o: %.cpp Makefile
	@mkdir -p $(@D)
	@$(ECHO) $(BUILDING) "$(patsubst %.cpp,%.o,$<)"
	@$(CC) $(FLAGS) -MMD -MP -c $< -o $@
	@$(ECHO) $(BUILDING) "$(patsubst %.cpp,%.d,$<)"
	@sleep 0.05

$(TARGET): $(OBJ)
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) $(LIBS) $^ -o $(TARGET)
	@$(ECHO) $(BUILDING) $(TARGET)


-include $(DEPS)

build:
	@$(STARTING)
	@mkdir -p $(OBJDIR)

clean:
	@$(STARTING)
	@$(ECHO) $(DELETING) "object files"
	@sleep 0.2
	@$(ECHO) $(DELETING) "dependency files"
	@sleep 0.2
	@$(ECHO) $(DELETING) "${TARGET}"
	@sleep 0.2
	@$(ECHO) $(DELETING) "$(OBJDIR)/"
	-@rm -rf $(OBJDIR) $(TARGET)
	@sleep 0.2
	@$(ECHO) $(FINISHED) "$(GRN)CLEANING $(RST)\n"

info:
	@echo -e "$(GRN)GENERAL:\n\
	$(YLW)   TARGET  | $(BLU)$(TARGET)$(RST)\n\
	$(YLW)   CC      | $(BLU)$(CC)$(RST)\n\
	$(YLW)   LIBS    | $(BLU)$(LIBS)$(RST)\n\
	$(YLW)   FLAGS   | $(BLU)$(FLAGS)$(RST)\n\
	$(YLW)   RELEASE | $(BLU)$(RELEASE)$(RST)\n\
	$(YLW)   DEBUG   | $(BLU)$(DEBUG)$(RST)\n "

	@echo -e "$(GRN)SOURCES:   $(BLU)\n   $(patsubst %.cpp,%.cpp \n  ,$(SRC))"
	@echo -e "$(GRN)INCLUDE:   $(BLU)\n   $(patsubst %.hpp,%.hpp \n  ,$(INC))"
	@echo -e "$(GRN)OBJECTS:   $(BLU)\n   $(patsubst %.cpp,%.o \n  ,$(SRC))"
	@echo -e "$(GRN)DEPENDS:   $(BLU)\n   $(patsubst %.cpp,%.d \n  ,$(SRC))"

.PHONY: all build debug release clean info
