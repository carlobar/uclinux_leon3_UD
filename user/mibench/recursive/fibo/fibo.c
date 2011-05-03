/*Print requested number of terms of the Fibonacci sequence*/
#include <stdio.h>
#include <stdlib.h>



int main(int argc, char *argv[])
{
__asm__ ("set	0x00000003, %o0\n\t"
	 "set	0x00000001, %o1\n\t"
	 "set	0x80000b00, %o2\n\t"
	 "st	%o0, [%o2 + 0x08]\n\t"
	 "st	%o1, [%o2 + 0x04]\n\t");


     int n,number;

printf("start %d \n",argc);     
     if (argc!=2){
          printf("Usage:fibo <n>\n");
          return 2;
     }

     n=atoi(argv[1]);
     if (n<1){
          printf("Requested number of elements is less than 1\n");
          return 2;
     }

	number = fibcalc(n-1);
	printf("%d\n",number);

__asm__ ("set	0x00000000, %o1\n\t"
	 "set	0x80000b00, %o2\n\t"
	 "st	%o1, [%o2 + 0x04]\n\t");

     return 0;

}


int fibcalc(int n)
{
    if (n <= 1)
    {
//        cout << n;
        return n;
    }
    else
    {
        return fibcalc(n-1)+fibcalc(n-2);
    }
}


