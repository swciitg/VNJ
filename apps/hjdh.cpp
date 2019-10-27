#include <iostream>
#include <cmath>
using namespace std;

int& func(int& a){

int i=a;

return i;
}

int main(){

int a = 16;
int& b = func(a);   
a = sqrt(sqrt(sqrt(sqrt(a))));

cout << b + a << endl;

return 0;
}
