#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define UNLIMIT
#define MAXARRAY 60000 /* this number, if too large, will cause a seg. fault!! */

struct myStringStruct {
  char qstring[128];
};

int compare(const void *elem1, const void *elem2)
{

__asm__ ("set	0x00000003, %o0\n\t"
	 "set	0x00000001, %o1\n\t"
	 "set	0x80000b00, %o2\n\t"
	 "st	%o0, [%o2 + 0x08]\n\t"
	 "st	%o1, [%o2 + 0x04]\n\t");




  int result;
  
  result = strcmp((*((struct myStringStruct *)elem1)).qstring, (*((struct myStringStruct *)elem2)).qstring);

  return (result < 0) ? 1 : ((result == 0) ? 0 : -1);
}


int
main(int argc, char *argv[]) {
  struct myStringStruct array[MAXARRAY];
  FILE *fp;
  int i,count=0;
  
  if (argc<2) {
    fprintf(stderr,"Usage: qsort_small <file>\n");
    exit(-1);
  }
  else {
    fp = fopen(argv[1],"r");
    
    while((fscanf(fp, "%s", &array[count].qstring) == 1) && (count < MAXARRAY)) {
	 count++;
    }
  }
  printf("\nSorting %d elements.\n\n",count);
  qsort(array,count,sizeof(struct myStringStruct),compare);
  
  for(i=0;i<count;i++)
    printf("%s\n", array[i].qstring);


__asm__ ("set	0x00000000, %o1\n\t"
	 "set	0x80000b00, %o2\n\t"
	 "st	%o1, [%o2 + 0x04]\n\t");

  return 0;
}
