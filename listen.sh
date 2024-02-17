#!/bin/bash

# A sample listener script, which will continuously read
# messages from the requested room (singular)

# Check if URL environment variable is set
if [ -z "$CAPI_URL" ]; then
    >&2 echo "Error: CAPI_URL environment variable is not set."
    exit 1
fi

# Check if token is set (it must be)
if [ -z "$CAPI_TOKEN" ]; then
    >&2 echo "Error: CAPI_TOKEN environment variable is not set."
    exit 1
fi

if [ "$#" -lt 1 ] 
then
    echo "Error: you must include at least the room number!"
    exit 1
fi

room=$1

if [ -z "$2" ]
then
   pull="30"
else
   pull="$2"
fi

# Prints the normal messages to stdout and prints the max mid to stderr
pymessage='
import csv
import sys
try:
   from colorama import init, Fore, Style
   init()
except:
   # No colorama...
   Fore = { 
      BLACK : "", RED : "", GREEN : "", YELLOW : "", 
      BLUE : "", MAGENTA : "", CYAN : "", WHITE: "" 
   }
   Back = Fore
   Style = { NORMAL : "", BRIGHT : "", DIM : "", RESET_ALL : "" }
reader = csv.reader(sys.stdin)
maxid = 0
for row in reader:
   
print(maxid, file=sys.stderr)
'

# The initial call pulls some amount, the later calls are an infinite loop.
# If the call fails, we wait for some time before trying again...
AUTH_HEADER="-H 'Authorization: Bearer $CAPI_TOKEN'"

curl -s $AUTH_HEADER -G \
   --data-urlencode "get=$pull" \
   --data-urlencode "rooms=$room" \
   "$CAPI_URL/small/chat" 

echo "DONE"
