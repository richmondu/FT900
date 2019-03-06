
OUTDIR = S
INDIR = .
RELDIR = ../AudioFiles

CRT_SRCS += \
response_44100.raw\
response_48k.raw\
response_32k.raw\
response_16k.raw\
response_8.raw

CRT_S += \
$(OUTDIR)/response_44100.S\
$(OUTDIR)/response_48k.S\
$(OUTDIR)/response_32k.S\
$(OUTDIR)/response_16k.S\
$(OUTDIR)/response_8k.S


#-------------------------------------------------------------------------------
# All targets
#-------------------------------------------------------------------------------
.PHONY: all
all: clean dirs $(CRT_S)

dirs: $(OUTDIR)

$(OUTDIR):
	@echo 'Creating directory: $<'
	mkdir $(OUTDIR)

# Each subdirectory must supply rules for building sources it contributes
$(OUTDIR)/%.S: $(INDIR)/%.raw
	@echo 'Building file: $<'
	@echo 'Making S file: $@'
	$(eval SYMNAME = $(subst .,_, $(notdir $<)))
	@echo 'Symbol: $(SYMNAME)'
	@echo ### Auto-generated file by AudioFiles.mk ### > $@
	@echo .global $(SYMNAME) >> $@
	@echo $(SYMNAME): >> $@
	@echo .incbin "$(RELDIR)/$<" >> $@
	@echo .byte 0 >> $@
	@echo .global $(SYMNAME)_end >> $@
	@echo $(SYMNAME)_end: >> $@
	@echo 'Finished making file: $@'
	@echo ' '
	sleep 2

.PHONY: clean
clean:
	-$(RM) $(CRT_S) 
	-@echo ' '
