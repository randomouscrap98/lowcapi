#!/usr/bin/env python

import os
import sys
import getpass
import urllib.parse
import urllib.request
from urllib.error import HTTPError

capiurl=os.environ.get("CAPI_URL")
capiexpire=os.environ.get("CAPI_TOKEN_EXPIRE", "31536000")

def perror(message, end=os.linesep):
    print(message, end=end, file=sys.stderr)
    sys.stderr.flush()

if not capiurl:
    perror("Error: CAPI_URL environment variable not set")
    exit(1)

params = {}
perror("Username: ", end="")
params["username"] = input()
perror("Password: ", end="")
params["password"] = getpass.getpass(prompt="")

encoded_params = urllib.parse.urlencode(params)
url = capiurl + "/small/login?" + encoded_params

try:
    response = urllib.request.urlopen(url)
    print(response.read().decode('utf-8'))
except HTTPError as ex:
    perror(ex.read().decode('utf-8'))
    exit(1)

