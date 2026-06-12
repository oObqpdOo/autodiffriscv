#include<math.h>
#include<stdio.h>
#include<stdlib.h>

#define ALPHA  0.16666667163372039794921875f
#define BETA   0.008333333767950534820556640625f
#define GAMMA  0.000198412701138295233249664306640625f
#define DELTA  0.000002755731884462875314056873321533203125f
#define PI	   3.1415927410125732421875f


float sine_kernel(float x){
	float x2 = x*x;
	float t1 = GAMMA - DELTA * x2;
	float t2 = BETA - x2 * t1;
	float t3 = ALPHA - x2 * t2;
	float t4 = 1 - x2 * t3;
	float y = x * t4;
	return y;
}
float sine(float x)
{
	float y;
	x = fmodf(x, 2*PI);
	float x_abs = fabsf(x);
	if(x_abs >= 0 && x_abs <= PI/2){	
		y = sine_kernel(x);
	}
	else if(x_abs > PI/2 && x_abs <= PI){
		y = sine_kernel(PI - x);
	}
	else if(x_abs > PI && x_abs <= 3*PI/2){
		y = -sine_kernel(x - PI);
	}
	else if(x_abs > 3*PI/2 && x_abs <= 2*PI){
		y = -sine_kernel(2*PI - x);
	}
	return y;
}

void test(float x){
	float y = sinf(x);
	float y_taylor = sine(x);
	float dy = cosf(x);
	//printf("sinf(%f) = %f\n", x, y);
	printf("sine(%f) = %f\n", x, y_taylor);
	printf("cos(%f) = %f\n", x, dy);	
	if(fabsf(y - y_taylor) <= 0.00001){
		printf("TEST PASSED\n\n");
	}
	else{
		printf("TEST FAILED\n\n");
	}
}

int main(){
/*
	test(0.1);
	test(0.2);
	test(0.3);
	test(0.4);
	test(0.5);
	test(0.6);
	test(0.7);
	test(0.8);	
	test(0.9);
*/
	test(1);
	test(2);
	test(3);
	test(4);
	test(5);
	test(6);
	test(7);
	test(8);
/*
	test(PI/4);
	test(PI/2);
	test(3*PI/4);
	test(4);
	test(5);
	test(6);
	test(7);
	test(42); */


	return EXIT_SUCCESS;
}
