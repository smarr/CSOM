#!/bin/sh
SCRIPT_PATH=`dirname $0`
exec node ${SCRIPT_PATH}/CSOM.js "$@"
