#!/bin/bash

# A sample send script, which takes whatever it 
# gets from stdin and sends it to your given room

if [ -z "$CAPI_URL" ]; then
    >&2 echo "Error: CAPI_URL environment variable is not set."
    exit 1
fi

if [ -z "$CAPI_TOKEN" ]; then
    >&2 echo "Error: CAPI_TOKEN environment variable is not set."
    exit 1
fi

if [ "$#" -lt 1 ]; then
    echo "Error: you must include at least the room number!"
    exit 1
fi

room=$1
read -p "Message in $room: " message

curl -s -H "Authorization: Bearer $CAPI_TOKEN" -G \
   --data-urlencode "message=$message" \
   "$CAPI_URL/small/post/$room"

