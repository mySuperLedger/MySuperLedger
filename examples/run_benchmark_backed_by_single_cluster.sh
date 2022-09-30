#!/bin/bash

WORKING_DIR=$(pwd)
echo "working dir=${WORKING_DIR}"

set -x

rm -rf node_0 && mkdir -p node_0/snapshots
./build/LedgerApp conf/benchmark_0.ini > node_0/log 2>&1 &
pgrep LedgerApp