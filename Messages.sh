#! /bin/sh
$EXTRACTRC */*.rc */*.ui >> rc.cpp || exit 11
$XGETTEXT `find . -name "*.cpp" -o -name "*.h"` -o $podir/kompare.pot
