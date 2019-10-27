#include<stdio.h>
void main()
{
	int a[20],i,m,n,b[10],loc;
	printf("enter size of array a");
	scanf("%d",&m);
	printf("enter size of array b");
	scanf("%d",&n);
	printf("enter loc to insert");
	scanf("%d",&loc);
	for(i=m-1;i>=loc;i--)
	{
		a[i+n]=a[i];
	}
	for(i=0;i<n;i++)
	{
		a[loc+i]=a[i];
	}
	a[i]!=0;
	printf("%d",a);
}
