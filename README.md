# Lowcapi
A set of scripts and tools to connect to the "small" endpoint in a contentapi instance. Useful for building your own basic chat.

## How to use
There are different implementations depending on your needs. Right now, these implementations are available:
- Python

Each one should implement a set of standard "commands" which you can use to interface with contentapi. These commands boil down to:
- **Auth**: send a username and password through stdin and get back a token through stdout.
- **Search**: send some search text through stdin and get back a list of matched rooms through stdout. Also reads the token from
  the `CAPI_TOKEN` environment variable if set (to search private rooms)
- **Send**: send a message through stdin to the room given on the command line and get the raw output back. You must set `CAPI_TOKEN`
  for this to function
- **Listen**: Continuously longpoll on the room given on the command line and produce the (potentially colored) output on stdout.
  You must set `CAPI_TOKEN`
For ALL commands, you must set `CAPI_URL`

## Simple implementation
There is an example which showcases the use of each of the commands, called `simple.sh` within the python folder. You can 
use it as the basis for your own chat, or simply use it as-is. If you want to skip the room question in simple.sh, set
`CAPI_ROOM` environment variable
