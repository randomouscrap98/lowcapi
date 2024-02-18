#!/usr/bin/env python3

import io
import os
import sys
import csv
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

params = {}
perror("Search: ", end="")
params["search"] = "%" + input() + "%"

# Set up request
encoded_params = urllib.parse.urlencode(params)
url = capiurl + "/small/search?" + encoded_params
req = urllib.request.Request(url)

if capitoken:
    perror("Using user token")
    req.add_header("Authorization", "Bearer " + capitoken)

response = urllib.request.urlopen(req)
reader = csv.reader(io.StringIO(response.read().decode('utf-8')))

# Print results
for row in reader:
   if "R" not in row[5]:
      private = "P"
   else:
      private = " "
   print("{}{:>7} - {}".format(private, row[6], row[0]))

