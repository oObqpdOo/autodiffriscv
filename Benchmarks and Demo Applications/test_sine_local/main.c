#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#define PI	  3.1415927410125732421875f

int main(){
	float x1 = PI/4;
	float x2 = PI/2;
	float x3 = 3*PI/4;
	float x4 = 0.1;
	float x5 = 4;
	float x6 = 5;
	float x7 = 42;


	float y;
	float dy;

	y = sinf(x1);
	dy = cosf(x1);

	printf("sin(%f) = %f\n", x1, y);
	printf("cos(%f) = %f\n", x1, dy);	

	y = sinf(x2);
	dy = cosf(x2);

	printf("sin(%f) = %f\n", x2, y);
	printf("cos(%f) = %f\n", x2, dy);

	y = sinf(x3);
	dy = cosf(x3);	

	printf("sin(%f) = %f\n", x3, y);
	printf("cos(%f) = %f\n", x3, dy);	

	y = sinf(x4);
	dy = cosf(x4);

	printf("sin(%f) = %f\n", x4, y);
	printf("cos(%f) = %f\n", x4, dy);	

	y = sinf(x5);
	dy = cosf(x5);

	printf("sin(%f) = %f\n", x5, y);
	printf("cos(%f) = %f\n", x5, dy);	

	y = sinf(x6);
	dy = cosf(x6);

	printf("sin(%f) = %f\n", x6, y);
	printf("cos(%f) = %f\n", x6, dy);

	y = sinf(x7);
	dy = cosf(x7);

	printf("sin(%f) = %f\n", x7, y);
	printf("cos(%f) = %f\n", x7, dy);

	return EXIT_SUCCESS;
}
