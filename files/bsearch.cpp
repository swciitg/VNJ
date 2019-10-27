#include<iostream>
using namespace std;
template<class t>
class binsearch
{
	public:
		t ar[10],key,first,last,n,i,mid;
		void bsearch();
};
template<class t>
void binsearch<t>::bsearch()
{
	cout<<"enter no. elements to be entered";
	cin>>n;
	cout<<"enter the elements";
	for(i=0;i<n;i++)
	{
		cin>>ar[i];
	}
	cout<<"enter the key value";
	cin>>key;
	if(first<=last)
	{
		first=0;
		last=n-1;
		mid=(first+last)/2;
		if(key==ar[mid])
		{
			cout<<"element found at"<<i<<"position";
			//break;
		}
		else if(key>ar[mid])
		{
			first=mid+1;
		}
		else
		{
			last=mid-1;
		}
	}
}
int main()
{
	binsearch<int>obj;
	obj.bsearch();
}
