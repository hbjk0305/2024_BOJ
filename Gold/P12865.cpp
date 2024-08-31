#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>


int N, K, items[2][101];	//weight, value
int ans[100001][101];	//row: weight_limit, col: item_limit
#define MAX(A,B) ((A)>(B)?(A):(B))
#define WEIGHT 0
#define VALUE 1
int main() {
	scanf("%d %d", &N, &K);
	for (int i = 1; i <= N; ++i) {
		scanf("%d %d", &items[WEIGHT][i], &items[VALUE][i]);
	}
	for (int i = 1; i <= K; ++i) {
		// weight limit is i.
		for (int j = 1; j <= N; ++j) {
			//item limit is j
			ans[i][j] = ans[i][j - 1];
			if (items[WEIGHT][j] <= i) {
				ans[i][j] = MAX(
					ans[i - items[WEIGHT][j]][j-1] + items[VALUE][j],
					ans[i][j]
				);
			}
		
		}
	}

	printf("%d", ans[K][N]);
}