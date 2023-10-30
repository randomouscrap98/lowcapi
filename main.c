#include <stdio.h>

#include "csv.h"
#include "config.h"

int main(int argc, char * argv[])
{
   struct LowcapiConfig config = lc_read_config(argc > 1 ? argv[1] : NULL);
   printf("api: %s\n", config.api);
   printf("initpull: %d\n", config.initpull);
   return 0;
}

