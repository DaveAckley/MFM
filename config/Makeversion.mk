####
# THE MFM SYSTEM VERSION IS DEFINED HERE
#
# Changes to this file should be committed alone and last, just prior
# to release, and then HEAD should be tagged appropriately:
# git tag -a -m "Tagging v3.0.5" v3.0.5
# git push origin v3.0.5

MFM_VERSION_MAJOR:=3
MFM_VERSION_MINOR:=0
MFM_VERSION_REV:=5


################## NOTHING BELOW HERE SHOULD NEED TO CHANGE ##################

MFM_VERSION_NUMBER:=$(MFM_VERSION_MAJOR).$(MFM_VERSION_MINOR).$(MFM_VERSION_REV)

# Get the repo version (and save it for possible non-git-repo builds downstream)
HAVE_GIT_DESCRIBE:=$(shell cd $(BASEDIR) && git describe 2>&1 >/dev/null && echo $$?)
ifeq ($(HAVE_GIT_DESCRIBE),0)
  MFM_TREE_VERSION:=$(shell cd $(BASEDIR) && git describe)
  $(shell echo "MFM_TREE_VERSION:=$(MFM_TREE_VERSION)" > TREEVERSION.mk)
else
  MFM_TREE_VERSION:=unknown-rev
  -include TREEVERSION.mk
endif
export MFM_TREE_VERSION
