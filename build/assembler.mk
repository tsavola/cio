ASCOMMAND	:= $(CC) $(GENERICFLAGS) $(ASFLAGS)

$(O)/obj/%.o: %.S
	$(call echo,Compile,$@)
	$(QUIET) mkdir -p $(dir $@)
	$(QUIET) $(ASCOMMAND) \
		-MF $(O)/obj/$(patsubst %,%.d,$*.S) \
		-MT $(O)/obj/$*.o -MT $(O)/obj/$*.os \
		-o $@ $*.S

$(O)/obj/%.os: %.S
	$(call echo,Compile,$@)
	$(QUIET) mkdir -p $(dir $@)
	$(QUIET) $(ASCOMMAND) $(PICFLAGS) \
		-MF $(O)/obj/$(patsubst %,%.d,$*.S) \
		-MT $(O)/obj/$*.o -MT $(O)/obj/$*.os \
		-o $@ $*.S
