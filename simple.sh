#!/bin/bash

set -e

if [ -z $CAPI_TOKEN ]
then
   CAPI_TOKEN=$(./auth.sh)
fi

if [ -z $CAPI_ROOM ]
then
   while true
   do
      ./search.sh
      read -p "Room choice (0 to search again): " CAPI_ROOM
      if [ "$CAPI_ROOM" != "0" ]
      then
         break
      fi
   done
fi

export CAPI_TOKEN
export CAPI_ROOM

./listen.sh $CAPI_ROOM


