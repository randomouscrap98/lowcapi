#include <stdlib.h>
#include <string.h>

#include "mycsv.h"

#define CSV_FIELDDELIM ','
#define CSV_ENCLOSE '"'

// NOTE: this implementation parses SPECIFICALLY the rfc csv files.
// Sorry Europe... https://datatracker.ietf.org/doc/html/rfc4180

static int csv_findend(char * here, char * end, struct CsvField * field)
{
   // End of a field, set next field
   if(here[0] == CSV_FIELDDELIM) {
      field->nextfield = here + 1; 
      return 1;
   }
   // Special end of a line, we accept it with JUST CR as end OR cr+nl
   else if(here[0] == '\r' && end - here >= 1 && here[1] == '\n') {
      field->nextline = here + 2;
      return 2;
   }

   return 0;
}

struct CsvField csv_parsefield(char * begin, char * end)
{
   struct CsvField field = { NULL, 0, 0, 0, NULL, NULL };

   //Initial character indicates what this is
   if(begin && begin < end)
   {
      //Special scan, enclosed field
      if(begin[0] == CSV_ENCLOSE)
      {
         field.escaped = 1;
         field.field = ++begin;

         //Do a special scan to find the end
         for(begin; begin < end; begin++)
         {
            //This may be the end of the field
            if(begin[0] == CSV_ENCLOSE) 
            {
               //This is NOT the end of a field, it's an escaped delimiter
               if(begin[1] == CSV_ENCLOSE) { begin++; }
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

// Copy and unescape the field into a malloc'd char array.
// The array may be slightly larger than necessary
char * csv_unescapefield(struct CsvField * field)
{
   //Potentially over-allocate to reduce complexity and compute
   char * result = malloc(sizeof(char) * (field->rawlen + 1));

   if(result)
   {
      //This was an empty field, just cap the end now
      if(!field->rawlen)
      {
         result[0] = 0;
      }
      else if(field->escaped)
      {
         char * next = result;

         for(int i = 0; i < field->rawlen; i++)
         {

         }
      }
      else
      {
         memcpy(result, field->field, sizeof(char) * field->rawlen);
         result[field->rawlen] = 0;
      }
   }

   return result;
}
