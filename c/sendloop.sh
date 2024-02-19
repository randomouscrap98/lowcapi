#!/bin/bash

if [ "$#" -lt 1 ]; then
    echo "Error: you must include at least the room number!"
    exit 1
fi

room=$1

while true
do
   file=$(mktemp)
   vim "$file"
   if [ ! -s "$file" ]
   then
      echo "File empty: exiting program"
      exit 2
   fi
   cat "$file" | ./lowcapi esend $room
   rm "$file"
done

