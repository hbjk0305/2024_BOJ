#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <Windows.h>

static unsigned long long seed = 5;
static int pseudo_rand(void)
{
	seed = seed * 25214903917ULL + 11ULL;
	return (seed >> 16) & 0x3fffffff;
}

/* These constant variables will NOT be changed */
static const long long PENALTY = 10'000'000'000'000LL;
static const int MAX_TC = 1;

const int MAXN = 1000;
const int MAXM = 1000;
const int ROAD_CNT = 100;

struct Coordinates
{
	int y, x;
	Coordinates()
	{
		y = x = 0;
	}
	Coordinates(int _y, int _x)
	{
		y = _y;
		x = _x;
	}
};
struct TrafficSignal
{
	int signal, next_signal;
	int y, x;

	TrafficSignal()
	{
		signal = next_signal = 0;
		y = x = -1;
	}
};

struct Vehicle
{
	int y, x, dir, dest_y, dest_x;
};

static Coordinates src[ROAD_CNT * 2], dest[ROAD_CNT * 2];
static int K, L;

static long long gTotalScore;

static int map[MAXM][MAXM];
static int map_bak[MAXM][MAXM];
static int crossroad_id[MAXM][MAXM];
static int crossroad_id_bak[MAXM][MAXM];
static TrafficSignal signal_list[ROAD_CNT * ROAD_CNT];
static Vehicle vehicles[MAXN];
static Vehicle vehicles_bak[MAXN];
static int vehicle_on_map[MAXM][MAXM];

static int dy[4] = { -1, 0, 1, 0 };
static int dx[4] = { 0, 1, 0, -1 };

static int is_movable[MAXN];
static int turn_dir[MAXN];

static int min_y, max_y, min_x, max_x;

static const int UP = 0;
static const int RIGHT = 1;
static const int DOWN = 2;
static const int LEFT = 3;

static const int VERTICAL = 1;
static const int HORIZONTAL = 2;
static const int UPLEFT = 3;
static const int RIGHTUP = 4;
static const int DOWNRIGHT = 5;
static const int LEFTDOWN = 6;

static const int TURN_LEFT = 3;
static const int GO_STRAIGHT = 0;
static const int TURN_RIGHT = 1;

////////////////////////////////////

extern void init(int N, Vehicle mVehicles[], int mMap[][MAXM], int mSignalID[][MAXM]);
extern bool process();

////////////////////////////////////

void change_signal(int cross_id, int next_signal)
{
	if (next_signal <= 0 || next_signal > 6)
		return;

	signal_list[cross_id].signal = 0;
	signal_list[cross_id].next_signal = next_signal;
}

static void make_tc()
{
	K = L = 0;
	min_y = MAXM - 1, min_x = MAXM - 1, max_y = 0, max_x = 0;

	for (int y = 0; y < MAXM; y++)
		for (int x = 0; x < MAXM; x++)
			crossroad_id[y][x] = map[y][x] = 0;

	for (int i = 0; i < ROAD_CNT; i++)
	{
		int dir = pseudo_rand() % 2;

		if (dir == 0) // HORIZONTAL
		{
			int y = pseudo_rand() % (MAXM - 200) + 100;

			if (map[y - 1][0] == 1 || map[y][0] == 1 || map[y + 1][0] == 1 || map[y + 2][0] == 1)
				continue;

			for (int x = 0; x < MAXM; x++)
			{
				map[y + 1][x] = map[y][x] = 1;
			}

			src[K] = Coordinates(y + 1, 0);
			dest[K++] = Coordinates(y, 0);
			src[K] = Coordinates(y, MAXM - 1);
			dest[K++] = Coordinates(y + 1, MAXM - 1);

			if (min_y > y)
				min_y = y;
			if (max_y < y)
				max_y = y;
		}
		else // VERTICAL
		{
			int x = pseudo_rand() % (MAXM - 200) + 100;

			if (map[0][x - 1] == 1 || map[0][x] == 1 || map[0][x + 1] == 1 || map[0][x + 2] == 1)
				continue;

			for (int y = 0; y < MAXM; y++)
			{
				map[y][x + 1] = map[y][x] = 1;
			}

			src[K] = Coordinates(0, x);
			dest[K++] = Coordinates(0, x + 1);
			src[K] = Coordinates(MAXM - 1, x + 1);
			dest[K++] = Coordinates(MAXM - 1, x);

			if (min_x > x)
				min_x = x;
			if (max_x < x)
				max_x = x;
		}
	}

	L = 0;
	for (int y = 100; y <= MAXM - 100; y++)
	{
		if (map[y - 1][0] == 0 && map[y][0] == 1)
		{
			for (int x = 100; x <= MAXM - 100; x++)
			{
				if (map[y - 1][x] == 1 && map[y - 1][x - 1] == 0)
				{
					L++;

					crossroad_id[y][x] = crossroad_id[y + 1][x] = crossroad_id[y][x + 1] = crossroad_id[y + 1][x + 1] = L;
					signal_list[L].signal = signal_list[L].next_signal = 1;
					signal_list[L].y = y;
					signal_list[L].x = x;
				}
			}
		}
	}

	for (int i = MAXN - 1; i >= 0; i--)
	{
		int src_idx = 0, dest_idx = 0;
		do
		{
			src_idx = pseudo_rand() % K;
			dest_idx = pseudo_rand() % K;
		} while (src_idx == dest_idx);

		vehicles[i].y = src[src_idx].y;
		vehicles[i].x = src[src_idx].x;
		vehicles[i].dest_y = dest[dest_idx].y;
		vehicles[i].dest_x = dest[dest_idx].x;

		if (vehicles[i].y < 100)
		{
			src[src_idx].y++;
			vehicles[i].dir = DOWN;
		}
		else if (vehicles[i].y > MAXM - 100)
		{
			src[src_idx].y--;
			vehicles[i].dir = UP;
		}
		else if (vehicles[i].x < 100)
		{
			src[src_idx].x++;
			vehicles[i].dir = RIGHT;
		}
		else
		{
			src[src_idx].x--;
			vehicles[i].dir = LEFT;
		}
	}

	for (int y = 0; y < MAXM; y++)
	{
		for (int x = 0; x < MAXM; x++)
		{
			map_bak[y][x] = map[y][x];
			crossroad_id_bak[y][x] = crossroad_id[y][x];
			vehicle_on_map[y][x] = 0;
		}
	}

	for (int i = 0; i < MAXN; i++)
	{
		vehicles_bak[i] = vehicles[i];
		vehicle_on_map[vehicles[i].y][vehicles[i].x] = 1;
	}
}

static int get_nextdir(Vehicle& tar)
{
	int dest_y = tar.dest_y;
	int dest_x = tar.dest_x;
	int dir = tar.dir;
	int next_y = tar.y + dy[dir];
	int next_x = tar.x + dx[dir];

	if (dest_y < next_y - 1)
	{
		if (dest_x < next_x - 1)
		{
			if (dir == RIGHT)
			{
				return TURN_LEFT;
			}
			else if (dir == DOWN)
			{
				return TURN_RIGHT;
			}
			else if (dir == UP)
			{
				if (next_y <= min_y + 1)
					return TURN_LEFT;
			}
			else if (dir == LEFT)
			{
				if (next_x <= min_x + 1)
					return TURN_RIGHT;
			}
		}
		else if (dest_x > next_x + 1)
		{
			if (dir == RIGHT)
			{
				if (next_x >= max_x - 1)
					return TURN_LEFT;
			}
			else if (dir == DOWN)
			{
				return TURN_LEFT;
			}
			else if (dir == UP)
			{
				if (next_y <= min_y + 1)
					return TURN_RIGHT;
			}
			else if (dir == LEFT)
			{
				return TURN_RIGHT;
			}
		}
		else
		{
			if (dir == RIGHT)
				return TURN_LEFT;
			else if (dir == LEFT)
				return TURN_RIGHT;
		}
	}
	else if (dest_y > next_y + 1)
	{
		if (dest_x < next_x - 1)
		{
			if (dir == RIGHT)
			{
				return TURN_RIGHT;
			}
			else if (dir == DOWN)
			{
				if (next_y >= max_y - 1)
					return TURN_RIGHT;
			}
			else if (dir == UP)
			{
				return TURN_LEFT;
			}
			else if (dir == LEFT)
			{
				if (next_x <= min_x + 1)
					return TURN_LEFT;
			}
		}
		else if (dest_x > next_x + 1)
		{
			if (dir == RIGHT)
			{
				if (next_x >= max_x - 1)
					return TURN_RIGHT;
			}
			else if (dir == DOWN)
			{
				if (next_y >= max_y - 1)
					return TURN_LEFT;
			}
			else if (dir == UP)
			{
				return TURN_RIGHT;
			}
			else if (dir == LEFT)
			{
				return TURN_LEFT;
			}
		}
		else
		{
			if (dir == RIGHT)
			{
				return TURN_RIGHT;
			}
			else if (dir == LEFT)
			{
				return TURN_LEFT;
			}
		}
	}
	else
	{
		if (dest_x < next_x - 1)
		{
			if (dir == DOWN)
			{
				return TURN_RIGHT;
			}
			else if (dir == UP)
			{
				return TURN_LEFT;
			}
		}
		else if (dest_x > next_x + 1)
		{
			if (dir == DOWN)
			{
				return TURN_LEFT;
			}
			else if (dir == UP)
			{
				return TURN_RIGHT;
			}
		}
	}
	return GO_STRAIGHT;
}

static void move_vehicles()
{
	for (int i = 0; i < MAXN; i++)
		is_movable[i] = 0;

	for (int i = 0; i < MAXN; i++)
	{
		int y = vehicles[i].y;
		int x = vehicles[i].x;
		int dir = vehicles[i].dir;
		int next_y = y + dy[dir];
		int next_x = x + dx[dir];
		int dest_y = vehicles[i].dest_y;
		int dest_x = vehicles[i].dest_x;

		if (y == dest_y && x == dest_x)
			continue;

		if (vehicle_on_map[next_y][next_x] == 1)
			continue;

		if (crossroad_id[next_y][next_x] == 0)
		{
			is_movable[i] = 1;
		}
		else
		{
			if (crossroad_id[y][x] == 0)
			{
				int signal = signal_list[crossroad_id[next_y][next_x]].signal;
				turn_dir[i] = get_nextdir(vehicles[i]);

				if (turn_dir[i] == TURN_RIGHT)
				{
					is_movable[i] = 1;
				}
				else
				{
					if (dir == UP && signal == UPLEFT)
						is_movable[i] = 1;
					else if (dir == RIGHT && signal == RIGHTUP)
						is_movable[i] = 1;
					else if (dir == DOWN && signal == DOWNRIGHT)
						is_movable[i] = 1;
					else if (dir == LEFT && signal == LEFTDOWN)
						is_movable[i] = 1;

					if (turn_dir[i] == GO_STRAIGHT)
					{
						if ((dir == UP || dir == DOWN) && (signal == VERTICAL))
							is_movable[i] = 1;
						else if ((dir == LEFT || dir == RIGHT) && signal == HORIZONTAL)
							is_movable[i] = 1;
					}
				}
			}
			else
			{
				is_movable[i] = 1;
			}
		}
	}

	for (int i = 0; i < MAXN; i++)
	{
		if (is_movable[i] == 0)
			continue;

		int dir = vehicles[i].dir;
		int y = vehicles[i].y;
		int x = vehicles[i].x;
		int next_y = y + dy[dir];
		int next_x = x + dx[dir];

		if (vehicle_on_map[next_y][next_x] == 0)
		{
			vehicles[i].y = next_y;
			vehicles[i].x = next_x;
			vehicle_on_map[y][x] = 0;
			vehicle_on_map[next_y][next_x] = 1;

			if (turn_dir[i] == TURN_RIGHT)
			{
				vehicles[i].dir = (vehicles[i].dir + 1) % 4;
				turn_dir[i] = GO_STRAIGHT;
			}
			else if (turn_dir[i] == TURN_LEFT)
			{
				int next_dir = (vehicles[i].dir + 3) % 4;
				if (next_dir == UP)
				{
					if (map[next_y + 1][next_x + 1] == 0)
					{
						vehicles[i].dir = next_dir;
						turn_dir[i] = GO_STRAIGHT;
					}
				}
				else if (next_dir == RIGHT)
				{
					if (map[next_y + 1][next_x - 1] == 0)
					{
						vehicles[i].dir = next_dir;
						turn_dir[i] = GO_STRAIGHT;
					}
				}
				else if (next_dir == DOWN)
				{
					if (map[next_y - 1][next_x - 1] == 0)
					{
						vehicles[i].dir = next_dir;
						turn_dir[i] = GO_STRAIGHT;
					}
				}
				else if (next_dir == LEFT)
				{
					if (map[next_y - 1][next_x + 1] == 0)
					{
						vehicles[i].dir = next_dir;
						turn_dir[i] = GO_STRAIGHT;
					}
				}
			}
		}
		if (vehicles[i].y == vehicles[i].dest_y && vehicles[i].x == vehicles[i].dest_x)
		{
			vehicle_on_map[vehicles[i].y][vehicles[i].x] = 0;
		}
	}

	for (int i = 1; i <= L; i++)
	{
		signal_list[i].signal = signal_list[i].next_signal;
	}
}

static bool verify()
{
	for (int i = 0; i < MAXN; i++)
	{
		if (vehicles[i].y != vehicles[i].dest_y || vehicles[i].x != vehicles[i].dest_x)
			return false;
	}
	return true;
}

void visualize(Vehicle &tar, int size) {
	for (int r = -size; r < size; ++r) {
		for (int c = -size; c < size; ++c) {
			if (r == 0 && c == 0) {
				printf("%d", tar.dir);
			}
			else if (crossroad_id[tar.y + r][tar.x + c]) {
				printf("%d", signal_list[crossroad_id[tar.y + r][tar.x + c]].signal);
			}
			else if (map[tar.y + r][tar.x + c] == 0) {
				printf("■");
			}
			else {
				printf("□");
			}
		}
		printf("\n");
	}
}


int main()
{
	setbuf(stdout, NULL);

	gTotalScore = 0;

	for (int tc = 0; tc < 10; ++tc) {
		make_tc();

		init(MAXN, vehicles_bak, map_bak, crossroad_id_bak);

		bool is_finished = false;
		while (is_finished == false)
		{
			is_finished = process();
			move_vehicles();
			gTotalScore++;
			
		}

		if (verify() == false) {
			gTotalScore = PENALTY;
			break;
		}
		printf("TC-%d: %lld\n", tc, gTotalScore);
	}

	long long SCORE = gTotalScore;
	printf("SCORE: %lld\n", SCORE);

	return 0;
}
