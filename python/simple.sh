#!/bin/bash

set -e

if [ -z $CAPI_TOKEN ]
then
   CAPI_TOKEN=$(./auth.py)
fi

export CAPI_TOKEN

if [ -z $CAPI_ROOM ]
then
   while true
   do
      ./search.py
      read -p "Room choice (0 to search again): " CAPI_ROOM
      if [ "$CAPI_ROOM" != "0" ]
      then
         break
      fi
   done
fi

export CAPI_ROOM

./listen.py $CAPI_ROOM


