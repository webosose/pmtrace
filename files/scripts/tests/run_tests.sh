#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
VIEWER=$DIR/../perf_log_viewer.py
CONF=$DIR/../../conf/perf-log-viewer-conf.json

LOGFILES=" \
    $DIR/000-launch-guide-messages \
    $DIR/001-settings-launch-messages \
    $DIR/002-applaunch-5sec-over-messages \
    $DIR/003-launchGuide-backToTv-returnToGuide-closeGuide-messages \
    $DIR/004-launchDiscovery-messages \
    $DIR/005-launchSettings-messages \
    $DIR/006-duplicated-getforegroundappinfo-messages"

echo "Viewer : $VIEWER"
echo "CONF   : $CONF"
echo ""

for i in $LOGFILES
do
    echo "Testing $i"
    $VIEWER -c $CONF -p $i | diff $i.expected -

    if [[ $? -ne 0 ]]; then
        echo "Failed"
    else
        echo "Passed"
    fi

    echo "--------------"
done
