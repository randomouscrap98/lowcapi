#!/bin/bash

set -e

if [ -z $CAPI_TOKEN ]
then
   CAPI_TOKEN=$(./lowcapi pauth)
fi

export CAPI_TOKEN

if [ -z $CAPI_ROOM ]
then
   while true
   do
      ./lowcapi psearch
      read -p "Room choice (0 to search again): " CAPI_ROOM
      if [ "$CAPI_ROOM" != "0" ]
      then
         break
      fi
   done
fi

export CAPI_ROOM

tmux \
   new-session "./lowcapi plisten $CAPI_ROOM" \; \
   split-window -h "./sendloop.sh $CAPI_ROOM"


