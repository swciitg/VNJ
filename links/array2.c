#include<stdio.h>
void main()
{
	int a[7]={7,13,21,16,9,31,24};
	int i,large;
	large=a[0];
	for(i=1;i<7;i++)
	{
		if(a[i]>large)
		{
			large=a[i];
		}
	}
	printf("%d",large);
}
