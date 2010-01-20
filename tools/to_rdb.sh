#!/bin/bash
#
# Convert a run output file into an RDB file.
#
# Takes 2 arguments: <rdb root dir> <datafile path>
# where the datafile name is in the format <alg>_<domain>_<inst num>.log
#
# The output will be in the appropriate plate in the RDB rooted at
# <rdb root dir>
#

FPATH=$2
ROOT_DIR=$1
FILE=$(basename $FPATH)
ALG=$(echo $FILE | awk -F_ '{print $1}')
DOMAIN=$(echo $FILE | awk -F_ '{print $2}')
NUM=$(echo $FILE | awk -F_ '{print $3}' | sed 's/.log//')
COST=$(grep g: $FPATH | sed 's/.*: \(.*\).*/\1/')
EXPANDED=$(grep expanded: $FPATH | sed 's/.*: \(.*\) (.*/\1/')
GENERATED=$(grep generated: $FPATH | sed 's/.*: \(.*\) (.*/\1/')
TIME=$(grep time: $FPATH | sed 's/.*: \(.*\) s.*/\1/')
CLOSED_SIZE=$(grep "closed size:" $FPATH | sed 's/.*: \(.*\).*/\1/')
CREATION_TIME=$(ls -l $FPATH | awk '{ print $6 " " $7 }')
LOOKUPS=$(grep lookup $FPATH \
    | sed 's/:/\t/' \
    | sed 's/lookups, /\t/' \
    | sed 's/hits (/\t/' \
    | sed 's/)//' \
    | sed 's/^[[:space:]]*/#altrow\t\"lookups\"\t/')

DOMAIN_DIR="$ROOT_DIR/$DOMAIN"
ALG_DIR="$DOMAIN_DIR/$ALG"

test -d $DOMAIN_DIR || {
    echo "Making $DOMAIN_DIR"
    mkdir $DOMAIN_DIR
}

test -d $ALG_DIR || {
    echo "Making $ALG_DIR"
    mkdir $ALG_DIR
    touch $ALG_DIR/../KEY=alg
    touch $ALG_DIR/KEY=num
}

(
echo -e "#start data file format 4"
echo -e "#altcols\t\"lookups\"\t\"lookup lvl\"\t\"lookup count\"\t\"hit count\"\t\"hit percentage\""
echo -e "$LOOKUPS"
echo -e "#pair\t\"alg\"\t\"$ALG\""
echo -e "#pair\t\"domain\"\t\"$DOMAIN\""
echo -e "#pair\t\"num\"\t\"$NUM\""
echo -e "#pair\t\"final sol cost\"\t\"$COST\""
echo -e "#pair\t\"total nodes expanded\"\t\"$EXPANDED\""
echo -e "#pair\t\"total nodes generated\"\t\"$GENERATED\""
echo -e "#pair\t\"total raw cpu time\"\t\"$TIME\""
echo -e "#pair\t\"closed size\"\t\"$CLOSED_SIZE\""
echo -e "#pair\t\"creation time\"\t\"$CREATION_TIME\""
echo -e "#end data file format 4"
) > $ALG_DIR/$NUM
