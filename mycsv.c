#include <stdlib.h>
#include <string.h>

#include "mycsv.h"

static int csv_findend(char * here, char * end, struct CsvField * field)
{
   // End of a field, set next field
   if(here[0] == ',' || here[0] == ';') {
      field->nextfield = here + 1; 
   }
   // End of a line, set next line
   else if(here[0] == '\n') { 
      field->nextline = here + 1; 
   }
   // Special end of a line, we accept it with JUST CR as end OR cr+nl
   else if(here[0] == '\r') {
      field->nextline = here + 1;
      if(end - here >= 1 && here[1] == '\n')
         field->nextline++; 
   }
   // What the hell is this
   else { 
      return 0;
   }
   return 1;
}

struct CsvField csv_parsefield(char * begin, char * end)
{
   struct CsvField field = { NULL, 0, 0, 0, NULL, NULL };

   //Initial character indicates what this is
   if(begin && begin < end)
   {
      //Special scan, enclosed field
      if(begin[0] == '"' || begin[0] == '\'')
      {
         char fieldend = begin[0];
         
         field.escaped = 1;
         field.field = ++begin;

         //Do a special scan to find the end
         for(begin; begin < end; begin++)
         {
            //This may be the end of the field
            if(begin[0] == fieldend) 
            {
               //This is NOT the end of a field, it's an escaped delimiter
               if(begin[1] == fieldend) { begin++; }
               //This IS the end of a field. Since we're here, we need to
               //figure out what's going on
               else {
                  //Try to find the end and set the appropriate fields.
                  if(!csv_findend(begin + 1, end, &field)) { field.error = CSVERR_BADFIELD; }
                  break;
               }
            }
            field.rawlen++;
         }
      }
      //Regular scan
      else
      {
         //Even if the field is empty, this should still work.
         field.field = begin;

         for(begin; begin < end; begin++)
         {
            if(csv_findend(begin, end, &field))
               break;
            field.rawlen++;
         }
      }
   }

   //Under no circumstances should we now be at the end of the file
   if(begin == end)
      field.error = CSVERR_BADFILE;

   return field;
}


