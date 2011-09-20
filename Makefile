CC=gcc
CFLAGS=-std=c99 -Wall -Werror -O2 -ggdb -pedantic
SRCDIR=./src
CSRC=$(wildcard $(SRCDIR)/*.c)
BUILDDIR=build
INCDIR=./include
INCDIRS=$(addprefix -I,$(INCDIR))
OBJ=$(patsubst %.c,%.o,$(addprefix $(BUILDDIR)/,$(notdir $(CSRC))))
TGT=goliath

ALL=$(TGT)

# creates a '.d' file in compiler output directories.
%/.d:
	@mkdir -p $(@D)
	@touch $@

.PRECIOUS: %/.d

# links goliath
$(TGT): $(OBJ)
	@echo LD $@
	@$(CC) -o $@ $(OBJ) $(LDFLAGS)

# builds object files in the $(BUILDDIR) directory.
$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(BUILDDIR)/.d
	@echo C $(basename $(notdir $@))
	@$(CC) $(CFLAGS) $(INCDIRS) -c $< -o $@

clean:
	@rm $(BUILDDIR)/*
	@rm goliath