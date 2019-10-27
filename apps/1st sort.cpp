#include<stdio.h>
#include<dirent.h>
int main()
{
	struct dirent**namelist;
	int n,i,alphasort;
	char pathname[100];
	scanf("%s",&pathname);
{

	public char* getcwd(char*,int pathname);
	n=(pathname,&namelist,0,alphasort);
}
	if(n<0)
	printf("error");
	else
	for(i=0;i<n;i++)
	printf("%s",namelist[i]->d_name);
}
