#!/usr/bin/env bash

# Hack to deal with macOS >= 10.11's per session shell history.
if [ $(uname) == "Darwin" ]; then
  SHELL_SESSION_HISTORY=0
  prev_shell_session_file=$SHELL_SESSION_FILE
  unset SHELL_SESSION_FILE
fi

function wd() {
  pushd $(dirname $BASH_SOURCE) >/dev/null
  echo $(pwd)
  popd >/dev/null
}

function libpath() {
  if [ $(uname) == "Darwin" ]; then
    echo DYLD_LIBRARY_PATH
  else
    echo LD_LIBRARY_PATH
  fi
}

prev_path=$PATH
eval prev_libpath=\$$(libpath)
prev_manpath=$MANPATH
prev_ps1=$PS1

export PREFIX=$(wd)/toolchain
export PATH=$PREFIX/bin:$PATH
export MANPATH=$PREFIX/share/man:$MANPATH
eval export $(libpath)=$PREFIX/lib:\$$(libpath)

if [[ ! -v MAKEFLAGS ]]; then
  makeflags_set=1
  export MAKEFLAGS=-j8
fi

PS1="(thimble $ARCH) $PS1"

function deactivate() {
  unset PREFIX
  export PATH=$prev_path
  eval export $(libpath)=$prev_libpath
  export MANPATH=$prev_manpath
  PS1=$prev_ps1

  if [[ -v makeflags_set ]]; then
    unset MAKEFLAGS
  fi

  unset prev_path
  unset prev_libpath
  unset prev_manpath
  unset prev_ps1
  unset makeflags_set
  unset deactivate
  unset ARCH
}

if [ $(uname) == "Darwin" ]; then
  unset SHELL_SESSION_HISTORY
  SHELL_SESSION_FILE=$prev_shell_session_file
  unset prev_shell_session_file
fi
