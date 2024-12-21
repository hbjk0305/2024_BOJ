#include <stdio.h>
#include <Windows.h>

#define MAXM 1000
#define ROAD_CNT 100
#define MIN(a, b) (((a)<(b))?(a):(b))
#define MAX(a, b) (((a)>(b))?(a):(b))

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

#define VERTICAL 1
#define HORIZONTAL 2
#define UPLEFT 3
#define RIGHTUP 4
#define DOWNRIGHT 5
#define LEFTDOWN 6

#define TURN_LEFT 3
#define GO_STRAIGHT 0
#define TURN_RIGHT 1

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

const int MAXN = 1000;
static int dy[4] = { -1, 0, 1, 0 };
static int dx[4] = { 0, 1, 0, -1 };

static int sdy[4] = { 2, 1, -1, 0 };
static int sdx[4] = { 1, -1, 0, 2 };
static Coordinates src[ROAD_CNT * 2], dest[ROAD_CNT * 2];
static int K, L, cnt;
static int min_y, max_y, min_x, max_x;

static int map[MAXM][MAXM];
static int crossroad_id[MAXM][MAXM];
static TrafficSignal signal_list[ROAD_CNT * ROAD_CNT];
static Vehicle vehicles[MAXN];
static int vehicle_on_map[MAXM][MAXM];
static int is_movable[MAXN];
static int turn_dir[MAXN];

extern void change_signal(int mCrossID, int mNextSignal);
static int get_nextdir(Vehicle& tar);
static void move_vehicles();

static void wrapper_change_signal(int cross_id, int next_signal) {
	if (next_signal <= 0 || next_signal > 6)
		return;

	signal_list[cross_id].signal = 0;
	signal_list[cross_id].next_signal = next_signal;
	change_signal(cross_id, next_signal);

}
void init(int N, Vehicle mVehicles[], int mMap[][MAXM], int mSignalID[][MAXM]) {
	cnt = N;
	K = L = 0;
	min_y = MAXM - 1, min_x = MAXM - 1, max_y = 0, max_x = 0;
	for (int i = 0; i < ROAD_CNT * ROAD_CNT; i++) {
		signal_list[i].x = signal_list[i].y = -1;
	}
	for (int r = 0; r < MAXM; r++) {
		for (int c = 0; c < MAXM; c++) {
			map[r][c] = mMap[r][c];
			vehicle_on_map[r][c] = -1;
		}
	}
	for (int i = 0; i < N; i++) {
		vehicles[i].y = mVehicles[i].y;
		vehicles[i].x = mVehicles[i].x;
		vehicles[i].dir = mVehicles[i].dir;
		vehicles[i].dest_y = mVehicles[i].dest_y;
		vehicles[i].dest_x = mVehicles[i].dest_x;
		vehicle_on_map[vehicles[i].y][vehicles[i].x] = i;
	}

	for (int r = 0; r < MAXM; r++) {
		for (int c = 0; c < MAXM; c++) {
			int sid = mSignalID[r][c];
			crossroad_id[r][c] = sid;
			if (sid && signal_list[sid].x==-1) {
				//처음 찾는 signal
				if (mSignalID[r + 1][c] == sid &&
					mSignalID[r][c + 1] == sid &&
					mSignalID[r + 1][c + 1]==sid) {
					signal_list[sid].y = r;
					signal_list[sid].x = c;
					signal_list[sid].signal = signal_list[sid].next_signal = 1;
					max_x = MAX(max_x, c);
					max_y = MAX(max_y, r);
					min_x = MIN(min_x, c);
					min_y = MIN(min_y, r);
					L = MAX(sid, L);
				}

			}
			
		}
	}
}

void process_signal(int sid) {
	if (signal_list[sid].signal == 0)
		return;
	int waited[7] = { 0 };
	for (int i = 0; i < 4; ++i) {
		int y = signal_list[sid].y + sdy[i];
		int x = signal_list[sid].x + sdx[i];
		int vid = vehicle_on_map[y][x];
		if (vid != -1) {
			//교차로 진입을 기다리는 차가 있음. 이 차가 원하는 신호는 뭘까?
			if (turn_dir[vid] == TURN_RIGHT)
				continue;
			else if (turn_dir[vid] == GO_STRAIGHT)
				waited[(i % 2) + 1]++;
			waited[i + 3]++;
		}
	}
	if (waited[signal_list[sid].signal])
		// 현재 신호로 지나갈 수 있는 차가 있음.
		return;
	int max = 0, idxmax = -1;
	for (int i = 1; i <= 6; ++i) {
		if (waited[i] > max) {
			max = waited[i];
			idxmax = i;
		}
	}
	if (max > 0) {
		wrapper_change_signal(sid, idxmax);
	}

}

bool process() {
	//printf("%d ", L);
	for (int i = 1; i <= L; i++) {
		process_signal(i);
	}
	move_vehicles();
	//printf("Car Left: %d\n", cnt);
	return cnt == 0;
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

		if (vehicle_on_map[next_y][next_x] != -1)
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

		if (vehicle_on_map[next_y][next_x] == -1)
		{
			vehicles[i].y = next_y;
			vehicles[i].x = next_x;
			vehicle_on_map[y][x] = -1;
			vehicle_on_map[next_y][next_x] = i;

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
			vehicle_on_map[vehicles[i].y][vehicles[i].x] = -1;
			--cnt;
		}
	}

	for (int i = 1; i <= L; i++)
	{
		signal_list[i].signal = signal_list[i].next_signal;
	}
}