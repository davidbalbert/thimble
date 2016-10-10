# Thimble - A small OS that doesn't do much

## Requirements

Thimble should work on Linux or OS X, though I've only tested it on OS X.

- Bash
- A C compiler
- Make
- Ruby (any version should do)

## Building and running

```
$ bash mktoolchain.bash   # Downloads and builds compiler, assembler and emulator. Only run this once.

$ source env.bash
(thimble) $ make qemu

# When you're done working on thimble, `deactivate` will restore your path.
# It's kind of like a python virtual env:
(thimble) $ deactivate
$
```

## Xv6

Thimble takes a lot of inspiration (including coding conventions and function naming) and some source code from [xv6](https://pdos.csail.mit.edu/6.828/2016/xv6.html). As Thimble has grown, it has become more distinct from xv6, but there are a lot of similar design decisions that should feel familiar to anyone who knows the xv6 codebase.

For the xv6 license, see `ACKNOWLEDGEMENTS`

# License

Thimble is copyright 2015-2016 David Albert and is available under the terms of the GNU GPLv3 or later. See COPYING for more info.
