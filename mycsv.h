#ifndef __LC_HEADER_MYCSV__
#define __LC_HEADER_MYCSV__

#define CSVERR_BADFIELD 1
#define CSVERR_BADFILE 2
#define CSVERR_BADPROGRAM 3

struct CsvField
{
   char * field;     //Set if field found, direct pointer to underlying data
   int rawlen;       //Raw length of the field, useful for malloc (may be oversized)
   int escaped;      //Whether this field is escaped
   int error;        //Whether a parse error occurred
   char * nextfield; //Pointer to next field, if it another field exists
   char * nextline;  //Pointer to next line, if this field ends at a line
};

struct CsvField csv_parsefield(char * begin, char * end);
char * csv_unescapefield(struct CsvField * field);
int csv_iteratefunc(char * begin, char * end, 
      int (*fieldfunc)(int, struct CsvField *, void *), 
      void * state);

struct CsvAnalysis
{
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

#endif
