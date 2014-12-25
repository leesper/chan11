#include <iostream>
#include <thread>
#include <chrono>
#include "../src/chan.h"
using namespace std;
using namespace chan11;

void worker(Chan<int> &jobs, Chan<int> &done)
{
	int j;
	while (jobs.recv(j) == esucc)
	{
		cout << "received job " << j << endl;
	}

	cout << "received all jobs" << endl;
	done.send(1);
}

int main()
{
	Chan<int> jobs = make_chan<int>(6);
	Chan<int> done = make_chan<int>();

	std::thread worker_thread(worker, std::ref(jobs), std::ref(done));
	worker_thread.detach();

	for (int i = 0; i <= 3; ++i)
	{
		cout << "main thread sleep 1 sec" << endl;
		this_thread::sleep_for(std::chrono::seconds(1));
		jobs.send(i);
	}
	jobs.close();
	cout << "send all jobs" << endl;

	if (jobs.send(6) == epipe)
		cout << "chan closed, can't send" << endl;

	int r;
	done.recv(r);
	cout << "done: " << r << endl;
	return 0;
}
