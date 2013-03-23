#!/usr/bin/env bash

set -o errexit

LATEST_URL=http://path.gtgross.com/latest.tar.gz

# from: http://www.snabelb.net/content/bash_support_function_check_dependencies
function deps {
  DEPENDENCIES="curl tar autoreconf make"

  deps_ok=YES
  for dep in $DEPENDENCIES
  do
    if ! which $dep &>/dev/null;  then
      echo -e "This script requires $dep to run but it is not installed"
      echo -e "If you are running ubuntu or debian you might be able to install $dep with the following  command"
      echo -e "\t\tsudo apt-get install $dep\n"
      deps_ok=NO
    fi
  done
  if [[ "$deps_ok" == "NO" ]]; then
    echo -e "Unmet dependencies ^"
    echo -e "Aborting!"
    exit 1
  else
    return 0
  fi
}

function download {
  curl $LATEST_URL | tar xvz
}

function install {
  autoreconf --install
  ./configure
  make

  mkdir -p ~/bin
  cp ./bin/path ~/bin/path

  ~/bin/path -i
}

deps

BUILD_DIR="$(mktemp -d)"
cd $BUILD_DIR

download

install

rm -rf $BUILD_DIR
