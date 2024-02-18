#!/usr/bin/env python

import io
import os
import sys
import csv
import time
import shutil
import textwrap
import urllib.parse
import urllib.request
from urllib.error import HTTPError

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

if len(sys.argv) > 2:
   pull = sys.argv[2]
else:
   pull = "30"


def request(endpoint, params=None):
   url = capiurl + endpoint
   if params:
      encoded_params = urllib.parse.urlencode(params)
      url += "?" + encoded_params
   req = urllib.request.Request(url)
   req.add_header("Authorization", "Bearer " + capitoken)
   response = urllib.request.urlopen(req)
   return response.read().decode('utf-8')

reader = csv.reader(io.StringIO(request("/small/me")))
for row in reader:
   username = row[1]

print("Listening as " + username)


mid = -1
lastuserlist = "???"

def capi_listen(getamnt):
   global mid
   params = {
      "rooms" : sys.argv[1],
      "get" : getamnt,
      "mid" : mid
   }
   while True:
      try:
         return request("/small/chat", params)
      except Exception as ex:
         perror("Listen error, sleeping for 10 seconds: " + str(ex))
         time.sleep(10)


def render(csvstr):
   global mid,lastuserlist
   reader = csv.reader(io.StringIO(csvstr))
   for row in reader:
      if row[4] == "userlist":
         if row[6] != "0" and lastuserlist != row[2]:
            print(Fore.YELLOW + "--Userlist: " + (row[2] or "EMPTY") + "\t" +
                  Style.DIM + Fore.MAGENTA + row[3] + Style.RESET_ALL)
            lastuserlist = row[2]
         continue
      elif row[4] == "eventId":
         continue
      if row[1] == username:
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
         if thisid > mid:
            mid = thisid


render(capi_listen("-" + pull))

while True:
   render(capi_listen(pull))

print("DONE")
