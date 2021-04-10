#!/usr/bin/env python3

import licant
import licant.install

licant.cxx_application("atom-node",
	srcdir="src",
	sources=["main.cpp"],
	include_paths=["src"],
	libs=["crow", "nos", "pthread", "igris", "notify"],
	cxx_flags = "-g $(pkg-config --cflags glib-2.0 gdk-pixbuf-2.0)",
	cc_flags = "-g $(pkg-config --cflags glib-2.0 gdk-pixbuf-2.0)",
)

licant.install.install_application(
	tgt="install", 
	src="atom-node", 
	dst="atom-node")


licant.ex("atom-node")