#include "snipmath.h"
#include <math.h>


int main(void)
{

__asm__ ("set	0x00000003, %o0\n\t"
	 "set	0x00000001, %o1\n\t"
	 "set	0x80000b00, %o2\n\t"
	 "st	%o0, [%o2 + 0x08]\n\t"
	 "st	%o1, [%o2 + 0x04]\n\t");

  double  a1 = 1.0, b1 = -10.5, c1 = 32.0, d1 = -30.0;
  double  a2 = 1.0, b2 = -4.5, c2 = 17.0, d2 = -30.0;
  double  a3 = 1.0, b3 = -3.5, c3 = 22.0, d3 = -31.0;
  double  a4 = 1.0, b4 = -13.7, c4 = 1.0, d4 = -35.0;
  double  x[3];
  double X,fl;
  int     solutions;
  int i,n_eq=0;
  unsigned long l = 0x3fed0169L;
  struct int_sqrt q;
  long n = 0;
  /* solve soem cubic functions */
  printf("********* CUBIC FUNCTIONS ***********\n");
  /* should get 3 solutions: 2, 6 & 2.5   */
  SolveCubic(a1, b1, c1, d1, &solutions, x);  
  printf("1st equation\n");
  for(i=0;i<solutions;i++)
    //printf(" %f",x[i]);  		////OJO
  //printf("\n");          		////OJO
  /* should get 1 solution: 2.5           */
  SolveCubic(a2, b2, c2, d2, &solutions, x);  
  printf("2nd equation\n");
  for(i=0;i<solutions;i++)
    //printf(" %f",x[i]);			////OJO
  //printf("\n");				////OJO
  SolveCubic(a3, b3, c3, d3, &solutions, x);
  printf("3rd equation\n");
  for(i=0;i<solutions;i++)
  //  printf(" %f",x[i]);			////OJO
  //printf("\n");				////OJO
  SolveCubic(a4, b4, c4, d4, &solutions, x);
  printf("4th equation\n");
  for(i=0;i<solutions;i++)
  //  printf(" %f",x[i]); 			////OJO
  //printf("\n");				////OJO
  /* Now solve some random equations */
printf("Solving many random equations\n");   		////OJO
  for(a1=1;a1<10;a1++) {
    for(b1=10;b1>0;b1--) {
      for(c1=5;c1<15;c1+=0.5) {
	for(d1=-1;d1>-11;d1--) {
	  SolveCubic(a1, b1, c1, d1, &solutions, x);  
	  n_eq = n_eq++;
	  //printf("Solutions: 5 (random)");    ////OJO
	  //for(i=0;i<solutions;i++)		////OJO
	    //printf(" %f",x[i]);		////OJO
	  //printf("\n");			////OJO
	}
      }
    }
  }
printf("Solved %d random cubic equations\n",n_eq);
  
  printf("******** 1000 INTEGER SQR ROOTS *********\n");
  /* perform some integer square roots */
  for (i = 0; i < 1001; ++i)
    {
      usqrt(i, &q);
			// remainder differs on some machines
     // printf("sqrt(%3d) = %2d, remainder = %2d\n",
     //printf("sqrt(%3d) = %2d\n",			////OJO
     //	     i, q.sqrt);				////OJO
    }
  usqrt(l, &q);
  //printf("\nsqrt(%lX) = %X, remainder = %X\n", l, q.sqrt, q.frac);
  //printf("\nsqrt(%lX) = %X\n", l, q.sqrt);		////OJO


  printf("** ANGLE CONVERSION DEG to RAD from 0 to 360 **\n");
  /* convert some rads to degrees */
  for (X = 0.0; X <= 360.0; X += 1.0)
   fl= deg2rad(X);          /////OJO
    //printf("%3.0f degrees = %.12f radians\n", X, deg2rad(X));    /////OJO
  puts("");
 printf("** ANGLE CONVERSION RAD to DEG from 0.0 to 2PI sampling 1e-6 **\n");
  for (X = 0.0; X <= (2 * PI + 1e-6); X += (PI / 180))
   fl=rad2deg(X);   ////OJO
    //printf("%.12f radians = %3.0f degrees\n", X, rad2deg(X));
  
  printf("Program exits successfully\n");

__asm__ ("set	0x00000000, %o1\n\t"
	 "set	0x80000b00, %o2\n\t"
	 "st	%o1, [%o2 + 0x04]\n\t");

  return 0;
}
