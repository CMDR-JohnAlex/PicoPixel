#!/bin/bash
while true; do
    if [ -e /dev/ttyACM0 ]; then
        sudo minicom -b 115200 -o -D /dev/ttyACM0
        exit
    else
        inotifywait -e create /dev
    fi
done
