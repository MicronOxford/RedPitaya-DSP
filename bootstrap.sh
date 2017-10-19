#!/bin/sh

## This script helps bootstrapping when checked out from git.

## Copyright (C) 2017 David Pinto <david.pinto@bioch.ox.ac.uk>
##
## Copying and distribution of this file, with or without modification,
## are permitted in any medium without royalty provided the copyright
## notice and this notice are preserved.  This file is offered as-is,
## without any warranty.

## Don't ignore failures.
set -e

echo "Dummy file for autoreconf.  Generate it later from git log." > ChangeLog

autoreconf --install
echo "bootstrap done.  Now you can run './configure'."
