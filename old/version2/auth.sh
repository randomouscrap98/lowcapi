#!/bin/bash

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

# NOTE: if your curl does not support --fail-with-body (7.76 or higher), 
# you can simply return the result of curl without it directly and skip
# all the stuff after it. However, it means you will not know if your 
# login failed over stderr

# Send the GET request using curl with URL-encoding
response=`curl --fail-with-body -s -G \
   --data-urlencode "username=$username" \
   --data-urlencode "password=$password" \
   --data-urlencode "expireSeconds=$CAPI_TOKEN_EXPIRE" \
   "$CAPI_URL/small/login"`

error=$?

# Selectively output to different places
if [ $error -ne 0 ]
then
   >&2 echo "$response"
   exit $error
fi

echo "$response"

