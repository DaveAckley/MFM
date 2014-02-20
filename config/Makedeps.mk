
# We need a kickstart rule to get .d files going, before their first real compile-time generation:
$(BUILDDIR)/%.d:	
	@mkdir -p $(BUILDDIR)

# After we make .d's, let's keep them
.PRECIOUS:	$(BUILDDIR)/%.d

# Suck in generated autodependencies
AUTODEPS:=$(wildcard $(BUILDDIR)/*.d)
-include $(AUTODEPS)
