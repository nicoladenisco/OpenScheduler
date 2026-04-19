#!/ bin / bash

mydir = "$(cd " $(dirname "$0") " && pwd)"

if["$mydir" == ""]; then
    echo "mancata definizione directory di build."
else
    echo "Run target in $mydir/debug" cd $mydir./
    debug/oskcli
    cd..
fi
