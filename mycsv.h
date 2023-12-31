#ifndef LC_HEADER_MYCSV
#define LC_HEADER_MYCSV

#define CSVERR_BADFIELD 1
#define CSVERR_BADFILE 2

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

#endif
