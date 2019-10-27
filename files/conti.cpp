#include<stdio.h>
#include<string.h>
struct filetable
{
 char name[20];
 int sb,nob;
}ft[30];
int main()
{
 int i,j,n;
 char s[20];
printf("Enter no. of files:");
 scanf("%d",&n);
 for(i=0;i<n;i++)
 {
printf("\n Enter the file name%d:",i+1);
  scanf("%s",ft[i].name);
printf("Enter starting block of file %d:",i+1);
  scanf("%d",&ft[i].sb);
printf("Enter no. of blocks in file %d:",i+1);
  scanf("%d",&ft[i].nob);
 }
printf("\n Enter filename to be searched :");
 scanf("%s",s);
 for(i=0;i<n;i++)
  if(strcmp(s,ft[i].name)==0)
   break;
  if(i==n)
printf("Filenot Found");
  else
  {
printf("\n filename \t startblock \t no. of blocks \t blocks occupied\n");
   printf("\n%s\t\t%d\t\t\t%d\t",ft[i].name,ft[i].sb,ft[i].nob);
   for(j=0;j<ft[i].nob;j++)
	printf("%d",ft[i].sb+j);
  }
}

