#include <thread>
#include <vector>
#include <iostream>
#include <chrono>
#include "../src/chan.h"
using namespace std;
using namespace chan11;

void rolling(vector<Chan<bool>> &channels)
{
	//while (true)
		for (auto &ch : channels)
		{
			ch.send(true);
		}
}

int main()
{
	vector<Chan<bool>> channels;
	for (auto i = 0; i != 6; ++i)
	{
		channels.push_back(make_chan<bool>());
	}
	std::thread rolling_thread(rolling, std::ref(channels));
	rolling_thread.detach();

	vector<shared_ptr<Case<bool>>> cases;
	for (auto ele : channels)
	{
		cases.push_back(make_shared<rCase<bool>>(ele));
	}

	int x;
	for (auto i = 0; i != 6; ++i)
	{
		switch (chan_select(cases))
		{
		case 0:
			if (cases[0]->get())
			{
				x = 1;
			}
			break;
		case 1:
			if (cases[1]->get())
			{
				x = 2;
			}
			break;
		case 2:
			if (cases[2]->get())
			{
				x = 3;
			}
			break;
		case 3:
			if (cases[3]->get())
			{
				x = 4;
			}
			break;
		case 4:
			if (cases[4]->get())
			{
				x = 5;
			}
			break;
		case 5:
			if (cases[5]->get())
			{
				x = 6;
			}
			break;
		default:
			x = -1;
			break;
		}
		cout << x << " ";
	}
	cout << endl;
	return 0;
}
