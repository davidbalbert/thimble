#!/usr/bin/env bash

function wd() {
  pushd $(dirname $BASH_SOURCE) >/dev/null
  echo $(pwd)
  popd >/dev/null
}

function libpath() {
  if [ $(uname) == Darwin ]; then
    echo DYLD_LIBRARY_PATH
  else
    echo LD_LIBRARY_PATH
  fi
}

prev_path=$PATH
eval prev_libpath=$`libpath`
prev_manpath=$MANPATH
prev_ps1=$PS1

export PREFIX=$(wd)/tools
export PATH=$PREFIX/bin:$PATH
export MANPATH=$PREFIX/share/man:$MANPATH
eval export `libpath`=$PREFIX/lib:$`libpath`

PS1="(thimble) $PS1"

function deactivate() {
  unset PREFIX
  export PATH=$prev_path
  eval export $(libpath)=$prev_libpath
  export MANPATH=$prev_manpath
  PS1=$prev_ps1
  unset deactivate
}

