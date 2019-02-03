#! /bin/sh
#

set -e
out="test-output-$$.txt"
trap 'rm -rf $out' INT QUIT TERM EXIT

prog="$1"
shift
ref="$1"
shift
args="$@"

"$prog" $args > "$out"
diff "$ref" "$out"
