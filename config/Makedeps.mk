# We need a kickstart rule to get .d files going, before their first
# real compile-time generation.  So make empty .d's, if they don't
# already exist.
$(BUILDDIR)/%.d:	$(BUILDDIR)
	@[ -e $@ ] || >$@

$(BUILDDIR):	
	@mkdir -p $@

# After we make .d's, let's keep them
.PRECIOUS:	$(BUILDDIR)/%.d

# Suck in generated autodependencies
AUTODEPS:=$(wildcard $(BUILDDIR)/*.d)
-include $(AUTODEPS)
