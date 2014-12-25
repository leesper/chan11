#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "../src/chan.h"
using namespace std;
using namespace chan11;

void thread_func1(Chan &ch)
{
	std::this_thread::sleep_for(std::chrono::seconds(3));
	ch.send(1);
}

void thread_func2(Chan &ch)
{
	std::this_thread::sleep_for(std::chrono::seconds(2));
	ch.send(2);
}

int main()
{
	Chan ch1 = make_chan();
	Chan ch2 = make_chan();
	std::thread thread1(thread_func1, std::ref(ch1));
	std::thread thread2(thread_func2, std::ref(ch2));
	thread1.detach();
	thread2.detach();

	rCase case1(ch1);
	rCase case2(ch2);
	vector<shared_ptr<Case>> cases = { make_shared<rCase>(ch1), make_shared<rCase>(ch2) };
	switch(chan_select(cases, true))
	{
	case 0:
		cout << "received from ch1: " << cases[0]->get() << endl;
		break;
	case 1:
		cout << "received from ch2: " << cases[1]->get() << endl;
		break;
	default:
		cout << "unblock" << endl;
		break;
	}
	return 0;
}
