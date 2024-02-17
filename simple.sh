#!/bin/bash

set -e

CAPI_TOKEN=$(./auth.sh)

while true
do
   ./search.sh
   read -p "Room choice (0 to search again): " room
   if [ "$room" != "0"]
   then
      break
   fi
done

echo "Room: $room"
