#include <iostream>
using namespace std;


int test_loop_invariant(int a, int b) {
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        int c = a * b;
        sum += c;
    }
    return sum;
}

int main(){
	int res = test_loop_invariant(5,7);
	cout<<res;
	return 0;
}
