#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "mycsv.h"

#define CSV_FIELDDELIM ','
#define CSV_ENCLOSE '"'

#define CSV_FIELDSALLOC 10

// NOTE: this implementation parses SPECIFICALLY the rfc csv files.
// Sorry Europe... https://datatracker.ietf.org/doc/html/rfc4180

// Look "here" for the end of a field or line. The appropriate pointers
// are set within the field if end is found. Returns nonzero for end
// found, 0 for no end found.
static inline int csv_findend(char * here, char * end, struct CsvField * field)
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

// Given a specific range to search in, and assuming "begin" is pointing to
// what is presumably the beginning of a new field, figure out the inner length
// of this field and pointers to the next field or line.
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
         for(; begin < end; begin++)
         {
            //This may be the end of the field
            if(begin[0] == CSV_ENCLOSE) 
            {
               //This is NOT the end of a field, it's an escaped delimiter
               if(begin[1] == CSV_ENCLOSE) { 
                  field.rawlen++;
                  begin++; 
               }
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

         for(; begin < end; begin++)
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
            *next = field->field[i];
            next++;

            //Assume the field is escaped properly (for speed)
            if(field->field[i] == CSV_ENCLOSE)
               i++;
         }

         *next = 0;
      }
      else
      {
         memcpy(result, field->field, sizeof(char) * field->rawlen);
         result[field->rawlen] = 0;
      }
   }

   return result;
}

// Main csv full parsing function. Given a range to search over (inclusive),
// this will repeatedly call your given "fieldfunc" for every field parsed out
// of the csv. No allocation is performed, you are given raw pointers into the 
// csv. The integer passed to your function is the field index, it resets to 0
// on a new line. You can also pass state to your function to avoid weird globals
int csv_iteratefunc(char * begin, char * end, 
      int (*fieldfunc)(int, struct CsvField *, void *), 
      void * state)
{
   int fieldnum = 0;
   char * next = begin;

   while(next < end)
   {
      struct CsvField field = csv_parsefield(next, end);

      if(field.error)
         return field.error;

      if(fieldfunc)
      {
         int result = fieldfunc(fieldnum, &field, state);
         if(result) return result;
      }

      if(field.nextline)
      {
         next = field.nextline;
         fieldnum = 0;
      }
      else if(field.nextfield)
      {
         next = field.nextfield;
         fieldnum++;
      }
      else
      {
         return CSVERR_BADPROGRAM;
      }
   }

   return 0;
}

// Simple wrapper for the common case of the entire csv being in memory
inline int csv_iteratefunc_f(char * csv, int (*fieldfunc)(int, struct CsvField *, void *), void * state)
{
   return csv_iteratefunc(csv, csv + strlen(csv) - 1, fieldfunc, state);
}

// An example per-field function, used for csv_analyze
static int csv_analyzefunc(int fieldnum, struct CsvField * field, void * state)
{
   struct CsvAnalysis * analysis = (struct CsvAnalysis *)state;

   analysis->totalfields++;
   analysis->totalfieldlength += field->rawlen;

   if(fieldnum == 0)
      analysis->lines++;

   if(analysis->lines == 1)
      analysis->columns = fieldnum + 1;

   if(field->escaped)
      analysis->escapedfields++;

   if(!field->rawlen)
      analysis->emptyfields++;
   else if(field->rawlen < analysis->smallestfieldlength)
      analysis->smallestfieldlength = field->rawlen;

   if(field->rawlen > analysis->largestfieldlength)
      analysis->largestfieldlength = field->rawlen;

   return 0;
}

// Inspect a csv and return various counts of fields, lines, etc
struct CsvAnalysis csv_analyze(char * begin, char * end)
{
   struct CsvAnalysis analysis = { 0 };
   analysis.smallestfieldlength = INT_MAX;

   analysis.error = csv_iteratefunc(begin, end, csv_analyzefunc, &analysis);

   return analysis;
}



void csv_freeline(struct CsvLine * line)
{
   if(line)
   {
      if(line->fields)
      {
         for(int i = 0; i < line->fieldcount; i++)
            free(line->fields[i]);

         free(line->fields);
      }

      free(line);
   }
}

struct CsvLineCursor csv_initcursor(char * begin, char * end)
{
   struct CsvLineCursor cursor = { 0, 0, NULL, begin, end };
   return cursor;
}

void csv_endcursor(struct CsvLineCursor * cursor)
{
   cursor->current = cursor->end;
   csv_freeline(cursor->line);
}

inline struct CsvLineCursor csv_initcursor_f(char * csv)
{
   return csv_initcursor(csv, csv + strlen(csv) - 1);
}


#define __MYCSV_EXITREADLINE(rerr) { cursor->error = rerr; csv_freeline(cursor->line); return NULL; }

// This function cleans up the lines it returns to you automatically, if 
// you call it continuously in a while loop. Thus, if you exit the while 
// loop early, you must clean up the line, otherwise all items will be
// cleaned up for you
struct CsvLine * csv_readline(struct CsvLineCursor * cursor)
{
   // Always free existing line. This is kinda bad, but this makes the common
   // code case very easy.
   csv_freeline(cursor->line);

   // Nothing to be done, just exit now
   if(cursor->current >= cursor->end)
      return NULL;

   // Initialize the line we'll be reading
   cursor->line = malloc(sizeof(struct CsvLine));
   if(!cursor->line) __MYCSV_EXITREADLINE(CSVERR_BADPROGRAM);
   cursor->line->start = cursor->current;
   cursor->line->fields = NULL;
   cursor->line->fieldcount = 0;
   cursor->line->fieldscapacity = 0;

   // Iterate until the end of the csv is reached
   while(cursor->current < cursor->end)
   {
      // Fix capacity pre-emptively
      if(cursor->line->fieldscapacity <= cursor->line->fieldcount)
      {
         size_t newcap = cursor->line->fieldscapacity + CSV_FIELDSALLOC;
         cursor->line->fields = realloc(cursor->line->fields, sizeof(char *) * newcap);
         if(!cursor->line->fields) __MYCSV_EXITREADLINE(CSVERR_BADPROGRAM); //Halt immediately
         cursor->line->fieldscapacity = newcap;
      }

      // Parse the next field
      struct CsvField field = csv_parsefield(cursor->current, cursor->end);
      if(field.error) __MYCSV_EXITREADLINE(field.error);

      // If there's no error, escape the current field.
      cursor->line->fields[cursor->line->fieldcount++] = csv_unescapefield(&field);

      // Field must have SOME kind of value for the next thing...
      if(field.nextline)
      {
         // We're at the end of a line, set pointer and return current line
         cursor->current = field.nextline;
         cursor->line->length = cursor->current - cursor->line->start;
         cursor->linecount++;
         return cursor->line;
      }
      else if(field.nextfield)
      {
         // More fields available, just move on
         cursor->current = field.nextfield;
      }
      else
      {
         // Huh? The field parsing is bad somehow
         __MYCSV_EXITREADLINE(CSVERR_BADPROGRAM);
      }
   }

   // Huh? How'd you get here? I guess if the end of a file is reached
   // before the end of a line and the field parser didn't report this as an
   // error, we will get here. If that's the case, big program error? Maybe?
   __MYCSV_EXITREADLINE(CSVERR_BADPROGRAM);
}

