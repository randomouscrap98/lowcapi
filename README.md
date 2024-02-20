# Lowcapi
A set of scripts and tools to connect to the "small" endpoint in a contentapi instance. 
Useful for building your own basic chat.

## How to use
There are different implementations depending on your needs. Right now, these implementations are 
available:
- C (recommended)
- Python

Each one should implement a set of standard "commands" which you can use to interface with 
contentapi. These commands boil down to:
- **Auth**: send a username and password through stdin and get back a token through stdout.
- **Search**: send some search text through stdin and get back a list of matched rooms through 
  stdout. Also reads the token from the `CAPI_TOKEN` environment variable if set (to search private 
  rooms)
- **Send**: send a message through stdin to the room given on the command line and get the raw 
  output back. You must set `CAPI_TOKEN` for this to function
- **Listen**: Continuously longpoll on the room given on the command line and produce the 
  (potentially colored) output on stdout.  You must set `CAPI_TOKEN`
For ALL commands, you must set `CAPI_URL`

## Simple implementation
There is an example which showcases the use of all the commands, called `simple.sh` within supported
folders (C, python, etc). Use it as the basis for your own chat, or simply use it as-is. If you 
want to skip the room question in simple.sh, set `CAPI_ROOM` environment variable. You can also set
CAPI_TOKEN to skip the login

## Important notes
### Prefer the C implementation
The python implementation was a proof of concept and is not kept as up to date
as the C implementation. It may be missing important features, or have bugs
that the C program does not. For instance, as of writing, the python program
does not set your avatar or the markup language on send (this might have been
fixed already). 

## Using the interface directly
You can actually use the `/api/small` interface directly, using command line
programs like curl or using whatever language you like. Although I recommend
against the python implementation, it does serve as a good example program
since it's significantly simpler (assuming it still works).

