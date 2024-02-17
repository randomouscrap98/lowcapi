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
mid=-1

if [ -z "$2" ]
then
   pull="30"
else
   pull="$2"
fi

# Check who the user is
username=$(curl -s -H "Authorization: Bearer $CAPI_TOKEN" -G \
   "$CAPI_URL/small/me" | python3 -c '
import csv
import sys
reader = csv.reader(sys.stdin)
for row in reader:
   print(row[1])
')

echo "Listening as $username"

# Prints the normal messages to stdout and prints the max mid to stderr
pymessage='
import csv
import sys
try:
   from colorama import init, Fore, Style
   init()
except:
   # No colorama...
   class Empty: 
      pass
   Fore = Empty()
   Back = Empty()
   Style = Empty()
   def colassign(o):
      o.BLACK=""
      o.RED=""
      o.GREEN=""
      o.YELLOW=""
      o.BLUE=""
      o.MAGENTA=""
      o.CYAN=""
      o.WHITE="" 
   colassign(Fore)
   colassign(Back)
   Style.NORMAL = ""
   Style.BRIGHT = ""
   Style.DIM = ""
   Style.RESET_ALL = ""
reader = csv.reader(sys.stdin)
maxid = 0
for row in reader:
   if row[4] in [ "userlist", "eventid" ]:
      continue
   print(Style.BRIGHT + Fore.GREEN + row[1] + "  " + Style.DIM + Fore.MAGENTA + row[3])
   print(Style.RESET_ALL + Fore.WHITE + row[2])
   if row[8]:
      thisid = int(row[8])
      if thisid > maxid:
         maxid = thisid
print(maxid, file=sys.stderr)
'

# The initial call pulls some amount, the later calls are an infinite loop.
# If the call fails, we wait for some time before trying again...

curl -s -H "Authorization: Bearer $CAPI_TOKEN" -G \
   --data-urlencode "get=-$pull" \
   --data-urlencode "rooms=$room" \
   --data-urlencode "mid=$mid" \
   "$CAPI_URL/small/chat" | python3 -c "$pymessage"

echo "DONE"
