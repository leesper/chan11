#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "../src/chan.h"
using namespace std;
using namespace chan11;

void thread_func1(Chan<int> &ch)
{
	std::this_thread::sleep_for(std::chrono::seconds(3));
	ch.send(1);
}

void thread_func2(Chan<int> &ch)
{
	std::this_thread::sleep_for(std::chrono::seconds(2));
	ch.send(2);
}

int main()
{
	Chan<int> ch1 = make_chan<int>();
	Chan<int> ch2 = make_chan<int>();
	std::thread thread1(thread_func1, std::ref(ch1));
	std::thread thread2(thread_func2, std::ref(ch2));
	thread1.detach();
	thread2.detach();

	vector<shared_ptr<Case<int>>> cases = { make_shared<rCase<int>>(ch1), make_shared<rCase<int>>(ch2) };
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
