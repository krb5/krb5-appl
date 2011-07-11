#!/bin/sh

# autogen.sh - Create or update the prebuilt autotools materials from
# a checked out source tree.  It is not necessarily to run this
# command when building from a release tarball.

# It is common for autogen scripts to check autotools versions or
# perform other safety checks.  Perhaps we will do that some day.
# For now just run the necessary commands.

autoheader
autoconf
rm -rf autom4te.cache
