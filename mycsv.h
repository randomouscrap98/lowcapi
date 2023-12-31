#ifndef __LC_HEADER_MYCSV__
#define __LC_HEADER_MYCSV__

#define CSVERR_BADFIELD 1
#define CSVERR_BADFILE 2
#define CSVERR_BADPROGRAM 3

#define CSV_EARLYSTOP 100

struct CsvField
{
   char * field;     //Set if field found, direct pointer to underlying data
   int rawlen;       //Raw length of the field, useful for malloc (may be oversized)
   int escaped;      //Whether this field is escaped
   int error;        //Whether a parse error occurred
   char * nextfield; //Pointer to next field, if it another field exists
   char * nextline;  //Pointer to next line, if this field ends at a line
};

//Parse the next field within the given range (both inclusive)
struct CsvField csv_parsefield(char * begin, char * end);

//Copy the parsed field into a malloc'd string, unescaping everything
char * csv_unescapefield(struct CsvField * field);

//The main csv processing function. Call the given fieldfunc for every field
//in the given range (both inclusive). Your given state will be passed to
//each call. You can determine a line start by first param = 0 in fieldfunc
int csv_iteratefunc(char * begin, char * end, 
      int (*fieldfunc)(int, struct CsvField *, void *), 
      void * state);

//Basic wrapper for common instance where the entire csv is in memory in 
//a standard null-terminated string
int csv_iteratefunc_f(char * csv,
      int (*fieldfunc)(int, struct CsvField *, void *), 
      void * state);

struct CsvAnalysis
{
   int error;
   int lines;
   int columns;
   int totalfields;
   int largestfieldlength;
   int smallestfieldlength;
   int emptyfields;
   int escapedfields;
   int totalfieldlength;
};

struct CsvAnalysis csv_analyze(char * begin, char * end);

// This is a representation of internal data passed to your line function.
// If you want anything out of it, you should COPY it, since the lifetimes
// of any pointer are not guaranteed after the call to your function.
struct CsvLine
{
   char ** fields;
   int fieldcount;
   int fieldscapacity;
};

// Frees a dynamically created line entirely, including the line itself
void csv_freeline(struct CsvLine * line);

////A slightly more basic function that iterates over lines, returning
////each one to your processing function already processed and ready to go.
////Note that you NEED to copy out any data you want to preserve from the 
////line data, as it is destroyed every iteration
//int csv_iteratelines(char * begin, char * end, 
//      int (*linefunc)(int, struct CsvLine *, void *), 
//      void * state);
//int csv_iteratelines_f(char * csv,
//      int (*linefunc)(int, struct CsvLine *, void *), 
//      void * state);

struct CsvLineCursor
{
   int linenumber;
   int error;
   struct CsvLine * line;
   char * current;
   char * end;
};

struct CsvLineCursor csv_initcursor(char * begin, char * end);
struct CsvLineCursor csv_initcursor_f(char * csv);

// Read each line using the given cursor. Will return NULL at end.
struct CsvLine * csv_readline(struct CsvLineCursor * cursor);

#endif
