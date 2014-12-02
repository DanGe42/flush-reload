#!/bin/sh

GPG=$HOME/gnupg-1.4.12/bin/gpg
ADDR=addr/osx.txt
MESSAGE=message.txt
OUT=out.txt
CYCLES=$1

bin/probe ${GPG} ${ADDR} ${OUT} ${CYCLES} &
PROBE_PID=$!

${GPG} --yes --sign ${MESSAGE} &
GPG_PID=$!

trap "echo 'Received signal'; kill -TERM ${PROBE_PID} ${GPG_PID}" \
    SIGINT SIGQUIT

wait ${PROBE_PID}
wait ${GPG_PID}

python graph.py &
GRAPH_PID=$!

trap "echo 'Killing python'; kill -TERM ${GRAPH_PID}" SIGINT SIGQUIT
wait ${GRAPH_PID}
