#!/bin/bash

# A sample listener script, which will continuously read
# messages from the requested room (singular)

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
mid=-1

if [ -z "$2" ]; then
   pull="30"
else
   pull="$2"
fi

# Check who the user is
export capi_username=$(curl -s -H "Authorization: Bearer $CAPI_TOKEN" -G \
   "$CAPI_URL/small/me" | python3 -c '
import csv
import sys
reader = csv.reader(sys.stdin)
for row in reader:
   print(row[1])
')

echo "Listening as $capi_username"

# Prints the normal messages to stdout and prints the max mid to stderr
pymessage='
import csv
import sys
import textwrap
import shutil
import os
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
   if row[4] == "userlist":
      if row[6] != "0":
         print(Fore.YELLOW + "Userlist: " + (row[2] or "EMPTY") + "\t" + 
            Style.DIM + Fore.MAGENTA + row[3] + Style.RESET_ALL)
      continue 
   elif row[4] == "eventId":
      continue
   if row[1] == os.environ["capi_username"]:
      uc = Fore.CYAN
   else:
      uc = Fore.GREEN
   print(Style.BRIGHT + uc + "[" + row[1] + "] ", end="")
   if "E" in row[5]:
      print(Style.RESET_ALL + Fore.RED + row[8] + "(edit)", end="")
   else:
      print(Style.DIM + Fore.WHITE + row[8], end="")
   print("\t" + Style.DIM + Fore.MAGENTA + row[3])
   lines = textwrap.fill(row[2].rstrip("\n"),
      width=shutil.get_terminal_size().columns - 2,
      replace_whitespace=False).split("\n")
   print(Style.RESET_ALL,end="")
   for line in lines:
      print(" " + line)
   if row[8]:
      thisid = int(row[8])
      if thisid > maxid:
         maxid = thisid
print(maxid, file=sys.stderr)
if maxid == 0:
   exit(55)
'

# ALL listens are mostly the same, luckily.
capi_listen() {
   while true
   do
      newmid=$(curl -s -H "Authorization: Bearer $CAPI_TOKEN" -G \
         --data-urlencode "get=$1" \
         --data-urlencode "rooms=$room" \
         --data-urlencode "mid=$mid" \
         "$CAPI_URL/small/chat" | python3 -c "$pymessage" 2>&1 >/dev/tty)

      err=$?

      if [ "$err" != "0" ]
      then
         >&2 echo "Listen error; waiting 10 seconds"
         sleep 10
      else
         break
      fi
   done

   if [ $newmid -gt $mid ]
   then
      mid=$newmid
   fi
}

# The initial call pulls some amount, the later calls are an infinite loop.
# If the call fails, we wait for some time before trying again...

capi_listen "-$pull"

while true
do
   capi_listen "$pull"
done

echo "DONE: $mid"
