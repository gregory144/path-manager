#!/usr/bin/env bash

set -o errexit

LATEST_URL=http://gtgross.com/path-manager/latest.tar.gz

# from: http://www.snabelb.net/content/bash_support_function_check_dependencies
function deps {
  DEPENDENCIES="git s3cmd"

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

git archive --format=tar.gz release >bin/latest.tar.gz

s3cmd put bin/latest.tar.gz scripts/install.sh s3://path.gtgross.com
