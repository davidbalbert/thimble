# Thimble - A small OS (maybe)

## Requirements

Thimble should work on Linux or OS X, though I've only tested it on OS X.

- Bash
- A C compiler
- GNUMake
- libsdl (for QEMU)

Running setup.bash will install a cross compiler in the tools directory.
Sourcing env.bash will put the cross compiler on your path. Running
`deactivate` in your shell restores things to normal (it's like a Python
virtual environment).
