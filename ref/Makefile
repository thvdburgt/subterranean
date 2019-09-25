CC = gcc
LD = gcc

DEBUG = true

CFLAGS += -std=c99
CFLAGS += -pedantic -Wall -Wextra -Wshadow -Wpointer-arith
CFLAGS  += -fsanitize=address,undefined
LDFLAGS := -fsanitize=address,undefined

ifeq ($(DEBUG), true)
	CFLAGS  += -DDEBUG -g -Og
else
	CFLAGS  += -DNDEBUG
	CFLAGS  += -O2
	# for stack size measurement:
	# CFLAGS  += -c -fdump-rtl-dfinish -fstack-usage -fno-stack-protector
endif

OBJDIR := .objs
DEPDIR := .builddeps
$(shell mkdir -p $(OBJDIR) >/dev/null)
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

EXECS ?= genkat_aead genkat_hash selftest

.PHONY: all
all: $(EXECS)

SELFTEST_SRCS = selftest.c test_crypto_aead.c crypto_aead.c subterranean.c subterranean-SAE.c
SELFTEST_OBJS = $(patsubst %,$(OBJDIR)/%.o,$(basename $(SELFTEST_SRCS)))
selftest: $(SELFTEST_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

GENKAT_AEAD_SRCS = genkat_aead.c test_crypto_aead.c crypto_aead.c subterranean-SAE.c subterranean.c
GENKAT_AEAD_OBJS = $(patsubst %,$(OBJDIR)/%.o,$(basename $(GENKAT_AEAD_SRCS)))
genkat_aead: $(GENKAT_AEAD_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

GENKAT_HASH_SRCS = genkat_hash.c crypto_hash.c subterranean-XOF.c subterranean.c
GENKAT_HASH_OBJS = $(patsubst %,$(OBJDIR)/%.o,$(basename $(GENKAT_HASH_SRCS)))
genkat_hash: $(GENKAT_HASH_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o : %.c
$(OBJDIR)/%.o : %.c $(DEPDIR)/%.d
	$(COMPILE.c) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))))

.PHONY: clean
clean:
	$(RM) -r $(OBJDIR)
	$(RM) -r $(DEPDIR)
	$(RM) $(EXECS)
	$(RM) LWC_AEAD_KAT_128_128.txt LWC_HASH_KAT_256.txt

