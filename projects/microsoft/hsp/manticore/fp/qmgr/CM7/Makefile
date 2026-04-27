# SPDX-License-Identifier: MIT
# Copyright (c) 2021-2026 Marvell

subdirs=Fps_cpu0 Fps_cpu1 Fps_cpu2

all:
	for d in $(subdirs); do make -C $$d $@ $(MAKEFLAGS) || exit 1; done

clean:
	for d in $(subdirs); do make -C $$d $@ $(MAKEFLAGS); done
