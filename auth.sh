#!/bin/bash

set -e

# An example authentication script, or you can use this one.
# This script briefly exposes your password on the process list.
# If this is a concern, just visit the url in your browser and
# get the token

# Check if URL environment variable is set
if [ -z "$CAPI_URL" ]; then
    >&2 echo "Error: CAPI_URL environment variable is not set."
    exit 1
fi

>&2 echo -n "Username: "
read username
>&2 echo -n "Password: "
read -s password

>&2 echo 

if [ -z "$CAPI_TOKEN_EXPIRE" ] 
then
    CAPI_TOKEN_EXPIRE="31536000"
fi

# Send the GET request using curl with URL-encoding
curl -s -G \
   --data-urlencode "username=$username" \
   --data-urlencode "password=$password" \
   --data-urlencode "expireSeconds=$CAPI_TOKEN_EXPIRE" \
   "$CAPI_URL/small/login"

>&2 echo

