#!/usr/bin/env python3

import licant

licant.cxx_application("atom-node",
	srcdir="src",
	sources=["main.cpp"],
	include_paths=["src"],
	libs=["crow", "nos", "pthread", "igris"],
)

licant.ex("atom-node")