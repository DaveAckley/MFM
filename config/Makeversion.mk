####
# THE MFM SYSTEM VERSION IS DEFINED HERE
#
# Changes to this file should be committed alone and last, just prior
# to release, and then HEAD should be tagged appropriately:
# git tag -a -m "Tagging v3.0.5" v3.0.5
# git push origin v3.0.5

MFM_VERSION_MAJOR:=3
MFM_VERSION_MINOR:=1
MFM_VERSION_REV:=3

################## NOTHING BELOW HERE SHOULD NEED TO CHANGE ##################

MFM_VERSION_NUMBER:=$(MFM_VERSION_MAJOR).$(MFM_VERSION_MINOR).$(MFM_VERSION_REV)

# Suck in a git rep marker if it's been cached
MFM_TREE_VERSION:=unknown-rev
-include $(BASEDIR)/MFM_TREEVERSION.mk

# If our dir is writable, and we have git, and there's a repo tag,
# that means we are in the MFM_REPO_BUILD_TIME era, so we should cache
# the tag for use in later eras.
SHOULD_CACHE_REPO_TAG_MFM:=$(shell test -w $(BASEDIR) && which git >/dev/null && cd $(BASEDIR) && git describe >/dev/null 2>&1 && echo YES)
#${info AT<<$(realpath $(BASEDIR))>>=($(SHOULD_CACHE_REPO_TAG_MFM))}
ifeq ($(SHOULD_CACHE_REPO_TAG_MFM),YES)
  MFM_TREE_VERSION:=$(shell cd $(BASEDIR) && git describe)
  $(shell echo "MFM_TREE_VERSION:=$(MFM_TREE_VERSION)" > $(BASEDIR)/MFM_TREEVERSION.mk)
else
endif
export MFM_TREE_VERSION
