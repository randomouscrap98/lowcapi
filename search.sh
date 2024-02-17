#!/bin/bash

set -e 

# An example search script, which includes some 
# portion of parsing the csv output. The search endpoint
# is incredibly simple in the api. You can do whatever
# you want with this; the searching is not necessary
# if you know the ID already

# Check if URL environment variable is set
if [ -z "$CAPI_URL" ]; then
    >&2 echo "Error: CAPI_URL environment variable is not set."
    exit 1
fi

>&2 echo -n "Search: "
read search

# Only do the header if the user set the thing (it's not required)
if [ -z "$CAPI_TOKEN" ]; then
    AUTH_HEADER=""
else
    AUTH_HEADER="-H 'Authorization: Bearer $CAPI_TOKEN'"
fi

# now the actual search! This uses python because apparently csv is 
# "too fancy" of a format or whatever (no standard stuff for it)
curl -s $AUTH_HEADER -G \
   --data-urlencode "search=%$search%" \
   "$CAPI_URL/small/search" | python3 printsearch.py

