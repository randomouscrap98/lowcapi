#!/usr/bin/env python3

import io
import os
import sys
import urllib.parse
import urllib.request

# User input setup
capiurl=os.environ.get("CAPI_URL")
capitoken=os.environ.get("CAPI_TOKEN")

def perror(message, end=os.linesep):
   print(message, end=end, file=sys.stderr)
   sys.stderr.flush()

if not capiurl:
   perror("Error: CAPI_URL environment variable not set")
   exit(1)

if not capitoken:
   perror("Error: CAPI_TOKEN environment variable not set")
   exit(1)

if len(sys.argv) < 2:
   perror("Error: must provide at least the room number!")
   exit(1)

room=sys.argv[1]

params = {}
perror("Message: ", end="")
params["message"] = input()

encoded_params = urllib.parse.urlencode(params)
url = capiurl + "/small/post/" + room + "?" + encoded_params
req = urllib.request.Request(url)
req.add_header("Authorization", "Bearer " + capitoken)

response = urllib.request.urlopen(req)
print(response.read().decode('utf-8'))
