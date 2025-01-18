#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>

static unsigned long long seed = 5;
static int pseudo_rand(void)
{
	seed = seed * 25214903917ULL + 11ULL;
	return (seed >> 16) & 0x3fffffff;
}

/* These constant variables will NOT be changed */
static const long long PENALTY = 1000'000'000'000LL;
static const int MAX_TC = 10;
static const int MAX_N = 100;
static const int MAX_BOX = 10'0000;

static int dx[6] = { 1, -1, 0, 0, 0, 0 };
static int dy[6] = { 0, 0, 1, -1, 0, 0 };
static int dz[6] = { 0, 0, 0, 0, 1, -1 };


struct Box {
	int destX;
	int destY;
	int weight;
};

static int gBoxHeight[MAX_N][MAX_N];
static Box gBoxInfo[MAX_BOX];
static Box* gBoxes[MAX_N][MAX_N][MAX_N];
static Box* gPickBox;
static int droneX, droneY, droneZ;

static long long SCORE = 0;
extern void process(void);


bool move(int dir)
{
	SCORE++;

	int nx = droneX + dx[dir];
	int ny = droneY + dy[dir];
	int nz = droneZ + dz[dir];

	if ((nx < 0) || (nx >= MAX_N) || (ny < 0) || (ny >= MAX_N) || (nz < 0) || (nz >= MAX_N))
		return false;

	if ((gPickBox == nullptr) && (gBoxHeight[ny][nx] > nz))
		return false;

	if ((gPickBox != nullptr) && (gBoxHeight[ny][nx] >= nz))
		return false;

	droneX = nx;
	droneY = ny;
	droneZ = nz;

	return true;
}

bool pick(int& destX, int& destY, int& weight)
{
	SCORE++;

	int& h = gBoxHeight[droneY][droneX];

	if (gPickBox != nullptr)
		return false;

	if (h != droneZ)
		return false;

	h--;
	gPickBox = gBoxes[h][droneY][droneX];

	destX = gPickBox->destX;
	destY = gPickBox->destY;
	weight = gPickBox->weight;

	return true;
}

bool drop()
{
	SCORE++;

	int& h = gBoxHeight[droneY][droneX];

	if (h != (droneZ - 1))
		return false;

	if (h == MAX_N - 2)
		return false;

	gBoxes[h][droneY][droneX] = gPickBox;
	gPickBox = nullptr;
	h++;

	return true;
}


void init()
{
	gPickBox = nullptr;
	droneX = droneY = droneZ = MAX_N - 1;

	for (int i = 0; i < MAX_N; i++) {
		for (int j = 0; j < MAX_N; j++) {
			gBoxHeight[i][j] = 0;
		}
	}

	for (int i = 0; i < MAX_BOX; i++) {
		int srcY = pseudo_rand() % MAX_N;
		int srcX = pseudo_rand() % MAX_N;
		int destY = pseudo_rand() % MAX_N;
		int destX = pseudo_rand() % MAX_N;
		int weight = pseudo_rand() % 1000 + 1;

		int& h = gBoxHeight[srcY][srcX];

		if (h == MAX_N - 2) {
			i--;
			continue;
		}

		gBoxInfo[i] = { destY, destX, weight };
		gBoxes[h][srcY][srcX] = &gBoxInfo[i];
		h++;
	}
}

bool verify()
{
	for (int y = 0; y < MAX_N; y++) {
		for (int x = 0; x < MAX_N; x++) {
			for (int h = 0; h < gBoxHeight[y][x]; h++) {
				if ((h > 0) && (gBoxes[h][y][x]->weight > gBoxes[h - 1][y][x]->weight))
					return false;

				if ((x != gBoxes[h][y][x]->destX) || (y != gBoxes[h][y][x]->destY))
					return false;
			}
		}
	}
	return true;
}

int main()
{
	setbuf(stdout, NULL);

	for (int i = 0; i < MAX_TC; i++) {

		init();
		process();

		if (verify() == false) {
			SCORE = PENALTY;
			break;
		}
	}

	// Expected score to pass -> under 250'000'000
	if (SCORE < 250'000'000)
		printf("PASS\n");
	else
		printf("SCORE: %lld\n", SCORE);

	return 0;
}