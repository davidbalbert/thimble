# Thimble - A small OS that doesn't do much

## Requirements

Thimble should work on Linux or OS X, though I've only tested it on OS X.

- Bash
- A C compiler
- Make
- libsdl (for QEMU)
- Ruby (any version should do)

## Building and running

```
$ bash setup.bash   # Downloads and builds cross compiler toolchain. Only run this once.

$ source env.bash
(thimble) $ make qemu

# When you're done working on thimble, `deactivate` will restore your path.
It's kind of like a python virtual env:
(thimble) $ deactivate
$
```

# License

Thimble is copyright 2015 David Albert and is available under the terms of the GNU GPLv3 or later. See COPYING for more info.
