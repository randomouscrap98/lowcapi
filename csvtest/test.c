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
   if(argc > 1)
   {
      basepath = argv[1];
   }
   else
   {
      fprintf(stderr, "Must provide the path to the test files\n");
      exit(1);
   }

   //The first arg is where the files are
   char * simple1 = loadfile("simple1.csv");
   struct CsvAnalysis analysis = analyze(simple1, 1);

   printf("Yeah: %d\n", analysis.lines);
}
