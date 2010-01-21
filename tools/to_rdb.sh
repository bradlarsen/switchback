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
DOMAIN=$(echo $FILE | sed 's/[^_]*_\([^0123456789]*\)_.*\.log/\1/')
NUM=$(echo $FILE | sed 's/[^0123456789]*_\(.*\)\.log/\1/')
CREATION_TIME=$(ls -l $FPATH | awk '{ print $6 " " $7 }')

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

test -e $ALG_DIR/$NUM && {
    echo "Skipping $ALG_DIR/$NUM"
    exit 0
}

if grep -q "found a solution:" $FPATH;
then
    echo "$ALG_DIR/$NUM: Solution found"
    COST=$(grep g: $FPATH | sed 's/.*: \(.*\).*/\1/')
    EXPANDED=$(grep expanded: $FPATH | sed 's/.*: \(.*\) (.*/\1/')
    GENERATED=$(grep generated: $FPATH | sed 's/.*: \(.*\) (.*/\1/')
    TIME=$(grep time: $FPATH | sed 's/.*: \(.*\) s.*/\1/')
    CLOSED_SIZE=$(grep -e "closed size:\|cache size:" $FPATH | sed 's/.*: \(.*\).*/\1/')
    if [[ "X$CLOSED_SIZE" == "X" ]];
    then
	CLOSED_SIZE=$(grep -e "nodes in closed" $FPATH | awk '{ print $1 }')
    fi
    LOOKUPS=$(grep lookup $FPATH \
	| sed 's/:/\t/' \
	| sed 's/lookups, /\t/' \
	| sed 's/hits (/\t/' \
	| sed 's/)//' \
	| sed 's/^[[:space:]]*/#altrow \"lookups\"\t/')

    (echo -e "#start data file format 4"
	if [[ "X$LOOKUPS" != "X" ]]
	then
	    echo -e "#altcols \"lookups\"\t\"lookup lvl\"\t\"lookup count\"\t\"hit count\"\t\"hit percentage\""
	    echo -e "$LOOKUPS"
	fi
	echo -e "#pair \"alg\"\t\"$ALG\""
	echo -e "#pair \"domain\"\t\"$DOMAIN\""
	echo -e "#pair \"num\"\t\"$NUM\""
	echo -e "#pair \"found solution\"\t\"yes\""
	echo -e "#pair \"final sol cost\"\t\"$COST\""
	echo -e "#pair \"total nodes expanded\"\t\"$EXPANDED\""
	echo -e "#pair \"total nodes generated\"\t\"$GENERATED\""
	echo -e "#pair \"total raw cpu time\"\t\"$TIME\""
	echo -e "#pair \"closed size\"\t\"$CLOSED_SIZE\""
	echo -e "#pair \"wall finish time\"\t\"$CREATION_TIME\""
	echo -e "#end data file format 4") > $ALG_DIR/$NUM
else				# no solution
    echo "$ALG_DIR/$NUM: No solution found"
    (echo -e "#start data file format 4"
	echo -e "#pair \"alg\"\t\"$ALG\""
	echo -e "#pair \"domain\"\t\"$DOMAIN\""
	echo -e "#pair \"num\"\t\"$NUM\""
	echo -e "#pair \"found solution\"\t\"no\""
	echo -e "#pair \"final sol cost\"\t\"infinity\""
	echo -e "#pair \"total nodes expanded\"\t\"infinity\""
	echo -e "#pair \"total nodes generated\"\t\"infinity\""
	echo -e "#pair \"total raw cpu time\"\t\"infinity\""
	echo -e "#pair \"closed size\"\t\"infinity\""
	echo -e "#pair \"wall finish time\"\t\"$CREATION_TIME\""
	echo -e "#end data file format 4") > $ALG_DIR/$NUM

fi
