#include "../mycsv.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define PRINTFILES
//#define PRINTANALYSIS

char * basepath;

char * loadfile(const char * filename)
{
   char path[512];
   sprintf(path, "%s/%s", basepath, filename);

   //Chatgpt
   FILE * file = fopen(path, "rb");  

   if (file == NULL) {
      perror("Error opening file");
      exit(1);
   }

   // Determine the file size
   fseek(file, 0, SEEK_END);
   long file_size = ftell(file);
   fseek(file, 0, SEEK_SET);

   // Allocate memory to store the entire file
   char *buffer = (char *)malloc(file_size + 1);
   if (buffer == NULL) {
      perror("Memory allocation failed");
      fclose(file);
      exit(1);
   }

   // Read the entire file into memory
   size_t read_size = fread(buffer, 1, file_size, file);
   fclose(file);

   if (read_size != (size_t)file_size) {
      perror("Error reading file");
      free(buffer);
      exit(1);
   }

   buffer[read_size] = 0;

#ifdef PRINTFILES
   printf("File %s:\n%s\n", path, buffer);
#endif

   return buffer;
}

struct CsvAnalysis analyze(char * filecontents, int error)
{
   struct CsvAnalysis analysis = csv_analyze(filecontents, filecontents + strlen(filecontents) - 1);
   assert(error == analysis.error);
#ifdef PRINTANALYSIS
   printf("l:%d c:%d t:%d\n", analysis.lines, analysis.columns, analysis.totalfields);
   printf("e:%d z:%d l:%d\n", analysis.escapedfields, analysis.emptyfields, analysis.totalfieldlength);
   printf("g:%d s:%d\n", analysis.largestfieldlength, analysis.smallestfieldlength);
   printf("-----------------------------\n");
#endif
   return analysis;
}


int main(int argc, char * argv[])
{
   //The first arg is where the files are
   if(argc > 1)
   {
      basepath = argv[1];
   }
   else
   {
      fprintf(stderr, "Must provide the path to the test files\n");
      exit(1);
   }

   char * file = loadfile("simple1.csv");
   struct CsvAnalysis analysis = analyze(file, 0);
   assert(analysis.lines == 3);
   assert(analysis.columns == 3);
   assert(analysis.totalfields == 9);
   assert(analysis.emptyfields == 0);
   assert(analysis.escapedfields == 0);
   assert(analysis.largestfieldlength == 8);
   assert(analysis.smallestfieldlength == 1);
   assert(analysis.totalfieldlength == 24);
   free(file);

   file = loadfile("simple2.csv");
   analysis = analyze(file, 0);
   assert(analysis.lines == 3);
   assert(analysis.columns == 3);
   assert(analysis.totalfields == 10);
   assert(analysis.emptyfields == 2);
   assert(analysis.escapedfields == 0);
   assert(analysis.largestfieldlength == 34);
   assert(analysis.smallestfieldlength == 1);
   assert(analysis.totalfieldlength == 57);
   free(file);

   file = loadfile("simple3.csv");
   analysis = analyze(file, 0);
   assert(analysis.lines == 1);
   assert(analysis.columns == 4);
   assert(analysis.totalfields == 4);
   assert(analysis.emptyfields == 0);
   assert(analysis.escapedfields == 0);
   assert(analysis.largestfieldlength == 6);
   assert(analysis.smallestfieldlength == 2);
   assert(analysis.totalfieldlength == 15);
   free(file);

   file = loadfile("escape1.csv");
   analysis = analyze(file, 0);
   assert(analysis.lines == 3);
   assert(analysis.columns == 3);
   assert(analysis.totalfields == 9);
   assert(analysis.emptyfields == 0);
   assert(analysis.escapedfields == 3);
   assert(analysis.largestfieldlength == 9);
   assert(analysis.smallestfieldlength == 1);
   assert(analysis.totalfieldlength == 23);
   free(file);

   file = loadfile("escape2.csv");
   analysis = analyze(file, 0);
   assert(analysis.lines == 2);
   assert(analysis.columns == 3);
   assert(analysis.totalfields == 6);
   assert(analysis.emptyfields == 0);
   assert(analysis.escapedfields == 5);
   assert(analysis.largestfieldlength == 10); //includes cr and lf
   assert(analysis.smallestfieldlength == 5);
   assert(analysis.totalfieldlength == 48);
   free(file);

   file = loadfile("unixend.csv");
   analysis = analyze(file, CSVERR_BADFILE);
   free(file);

   file = loadfile("badescape1.csv");
   analysis = analyze(file, CSVERR_BADFIELD);
   free(file);

   // --------------------------------------
   //  and now we test just the line thing
   // --------------------------------------
   file = loadfile("escape1.csv");
   struct CsvLineCursor cursor = csv_initcursor_f(file);
   while(csv_readline(&cursor))
   {
      struct CsvLine * line = cursor.line; 
      printf("%d: %s, %s, %s\n", cursor.linenumber, line->fields[0], line->fields[1], line->fields[2]);

      assert(cursor.linenumber >= 0 && cursor.linenumber < 3);
      assert(line->fieldcount == 3);
      assert(line->fieldscapacity > 3);

      if(cursor.linenumber == 0)
      {
         assert(strcmp(line->fields[0], "a") == 0);
         assert(strcmp(line->fields[1], "b") == 0);
         assert(strcmp(line->fields[2], "c") == 0);
      }
      else if(cursor.linenumber == 1)
      {
         assert(strcmp(line->fields[0], "this") == 0);
         assert(strcmp(line->fields[1], "that") == 0);
         assert(strcmp(line->fields[2], "the other") == 0);
      }
      else if(cursor.linenumber == 2)
      {
         assert(strcmp(line->fields[0], "9") == 0);
         assert(strcmp(line->fields[1], "8") == 0);
         assert(strcmp(line->fields[2], "7") == 0);
      }
   }

   free(file);

   printf("All pass\n");
}
