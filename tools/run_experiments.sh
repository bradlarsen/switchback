#!/bin/bash

set -e # exit on simple errors (?)


MEM_LIMIT=49283072  # 47 GB
#MEM_LIMIT=6291456  # 6 GB

TIME_LIMIT=900  # 15 minutes

TILES_DIR="${HOME}/hg/switchback/testdata/korf100"
SEARCH="${HOME}/hg/switchback/build/spacerelease/search"

LOG_DIR="${HOME}/hg/switchback/hastar_run_log"

ALGORITHMS="hastar"


get_run_info ()
{
    echo "start time: `date`"
    echo "mercurial revision: `hg id -n`"
    echo "system: `hostname`"
}


get_log_filename ()
{
    local domain=$1
    local algorithm=$2
    local instance=$3

    echo "${LOG_DIR}/${algorithm}_${domain}_${instance}.log"
}


run_korf_instance ()
{
    local algorithm=$1
    local instance_num=$2

    local logfile=$(get_log_filename "korf100" "$algorithm" "$instance_num")
    local instancefile="${TILES_DIR}/${instance_num}"
    "$SEARCH" "${algorithm}" "${instancefile}" 2>&1 | tee "$logfile"
}



############################################################
# MAIN
############################################################
mkdir -p "$LOG_DIR"

echo "$(get_run_info)" > "$LOG_DIR/run_info.log"

for algorithm in $ALGORITHMS; do
    for instance in `seq 1 100`; do
        (
            echo "########################################"
            echo "# Running $algorithm on Korf #$instance"
            echo "########################################"
            ulimit -v $MEM_LIMIT
            ulimit -t $TIME_LIMIT
            run_korf_instance $algorithm $instance
        )
    done
done

echo "stop time: `date`" >> "$LOG_DIR/run_info.log"
