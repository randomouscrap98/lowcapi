#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "mycsv.h"

#define CSV_FIELDDELIM ','
#define CSV_ENCLOSE '"'

#define CSV_FIELDSALLOC 10

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
            if(field->field[i] == CSV_FIELDDELIM)
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

int csv_iteratefunc_f(char * csv, int (*fieldfunc)(int, struct CsvField *, void *), void * state)
{
   return csv_iteratefunc(csv, csv + strlen(csv) - 1, fieldfunc, state);
}

// An example per-field function.
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

struct CsvAnalysis csv_analyze(char * begin, char * end)
{
   struct CsvAnalysis analysis = { 0 };
   analysis.smallestfieldlength = INT_MAX;

   analysis.error = csv_iteratefunc(begin, end, csv_analyzefunc, &analysis);

   return analysis;
}


void csv_resetline(struct CsvLine * line)
{
   if(line->fields)
   {
      for(int i = 0; i < line->fieldcount; i++)
         free(line->fields[i]);

      free(line->fields);
      line->fields = NULL;
   }

   //Just in case you want to reuse it or something. The line itself isn't freed
   line->fieldcount = 0;
   line->fieldscapacity = 0;
}

//Holder for line data as the line thing iterates.
struct CsvLineWrapper
{
   struct CsvLine line;
   int linenumber;
   int totalfields;
   void * userstate;
   int (*linefunc)(int, struct CsvLine *, void *);
};

static int csv_iteratelines_func(int fieldnum, struct CsvField * field, void * state)
{
   struct CsvLineWrapper * wrapper = (struct CsvLineWrapper *)state;
   struct CsvLine * line = &wrapper->line;

   //Start of a new line, call with the previous line data
   if(fieldnum == 0)
   {
      if(wrapper->totalfields)
         wrapper->linefunc(wrapper->linenumber++, line, wrapper->userstate);
      csv_resetline(line);
   }

   //Need to increase size again
   if(line->fieldscapacity <= line->fieldcount)
   {
      size_t newcap = line->fieldscapacity + CSV_FIELDSALLOC;
      line->fields = realloc(line->fields, sizeof(char *) * newcap);
      if(!line->fields) return CSVERR_BADPROGRAM; //Halt immediately
      line->fieldscapacity = newcap;
   }

   //"unescape" the field and put it in the array. This mallocs a new string
   //but it's cleaned up anytime the wrapper 
   line->fields[line->fieldcount++] = csv_unescapefield(field);

   wrapper->totalfields++;

   return 0;
}

int csv_iteratelines(char * begin, char * end, 
      int (*linefunc)(int, struct CsvLine *, void *), 
      void * state)
{
   struct CsvLineWrapper wrapper = {
      { NULL, 0, 0 }, 0, 0, state, linefunc
   };

   //Iterate over every field. Also at the end, you have to call the line
   //function for the very last line, since it won't be called during iteration
   int error = csv_iteratefunc(begin, end, csv_iteratelines_func, &wrapper);
   if(!error) linefunc(wrapper.linenumber, &wrapper.line, state);

   //Just always get rid of leftover data
   csv_resetline(&wrapper.line);

   return error;
}

int csv_iteratelines_f(char * csv, int (*linefunc)(int, struct CsvLine *, void *), void * state)
{
   return csv_iteratelines(csv, csv + strlen(csv) - 1, linefunc, state);
}
