#include "../mycsv.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * basepath;

char * loadfile(const char * filename)
{
   char path[512];
   sprintf(path, "%s/%s", basepath, filename);

   //Chatgpt
   FILE * file = fopen(path, "r");  

   if (file == NULL) {
      perror("Error opening file");
      exit(1);
   }

   // Determine the file size
   fseek(file, 0, SEEK_END);
   long file_size = ftell(file);
   rewind(file);

   // Allocate memory to store the entire file
   char *buffer = (char *)malloc(file_size);
   if (buffer == NULL) {
      perror("Memory allocation failed");
      fclose(file);
      exit(1);
   }

   // Read the entire file into memory
   size_t read_size = fread(buffer, 1, file_size, file);

   if (read_size != (size_t)file_size) {
      perror("Error reading file");
      free(buffer);
      fclose(file);
      exit(1);
   }

   fclose(file);

   return buffer;
}

struct CsvAnalysis analyze(char * filecontents, int error)
{
   struct CsvAnalysis analysis = csv_analyze(filecontents, filecontents + strlen(filecontents) - 1);
   assert(error == analysis.error);
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

   char * simple1 = loadfile("simple1.csv");
   struct CsvAnalysis analysis = analyze(simple1, 0);
   assert(analysis.lines == 3);
   assert(analysis.columns == 3);
   assert(analysis.totalfields == 9);
   assert(analysis.emptyfields == 0);
   assert(analysis.escapedfields == 0);
   assert(analysis.largestfieldlength == 8);
   assert(analysis.smallestfieldlength == 1);
   assert(analysis.totalfieldlength == 24);

   printf("All pass\n");
}
