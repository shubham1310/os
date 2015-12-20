#include <bits/stdc++.h>
using namespace std;

int main(){
	unsigned long long N, x, y, ans;
	scanf("%llu", &N);
	while(N--){
		scanf("%llu", &x);
		scanf("%llu", &y);
		if(x!=y && y+2!=x)
			printf("No Number\n");
		else if(x==y){
			if(x%2 == 0){
				ans = x+y;
				printf("%llu\n", ans);
			}
			else if(x%2 != 0){
				ans = x+y-1;
				printf("%llu\n", ans);
			}

		}
		else if(y+2 == x){
			if(y%2 == 0){
				ans = x+y;
				printf("%llu\n", ans);
			}
			else if(y%2 != 0){
				ans = x+y-1;
				printf("%llu\n", ans);
			}
		}

	}
}