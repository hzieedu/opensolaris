#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#

#
# Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# ident	"%Z%%M%	%I%	%E% SMI"
#

include ../Makefile.com

FILES += prototype_sparc

.KEEP_STATE:

all: $(FILES) depend

install: prototype_sparc all pkg

# FPScrubber consists of a generic SMF managed daemon and a hardware
# specific fpu test. Currently the test part is not built under gcc 
# compilation due to dependency on SunStudio libraries namely, 
# libsunperf.a, libfui.a, libfai.a, libfsu.a and libsunmath.a . So, 
# even when the build succeeds, for gcc based builds the test will 
# not be there. Therefore we make the package conditionally. 
# "proto_cc_sparc" prototype file contains objects for cc package
# and "proto_gcc_sparc" contains objects for gcc package.

proto:sh = \
	if [ -f $ROOT/usr/lib/fps/sun4u/UltraSPARC-III/fptest ]; \
	then \
	echo "prototype_cc_sparc"; \
	else \
	echo "prototype_gcc_sparc"; \
        fi

prototype_sparc: prototype_cc_sparc prototype_gcc_sparc
	$(RM) prototype_sparc
	@$(CP) $(proto) prototype_sparc


include ../Makefile.targ

