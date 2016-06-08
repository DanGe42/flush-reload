#!/bin/bash

GPG=gnupg-1.4.12/g10/gpg
ADDR=addr/linu.txt
MESSAGE=message.txt
OUT=out.txt
CYCLES=$1
KEY=Flush

bin/probe ${GPG} ${ADDR} ${OUT} ${CYCLES} &
PROBE_PID=$!

sleep 0.01
(echo "GPG start"; ${GPG} -u ${KEY} --yes --sign ${MESSAGE}; echo "GPG end") &
GPG_PID=$!

trap "echo 'Received signal'; kill -TERM ${PROBE_PID} ${GPG_PID}" \
    SIGINT SIGQUIT

wait ${PROBE_PID}
wait ${GPG_PID}

python graph.py &
GRAPH_PID=$!

trap "echo 'Killing python'; kill -TERM ${GRAPH_PID}" SIGINT SIGQUIT
wait ${GRAPH_PID}
