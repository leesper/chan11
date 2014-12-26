chan11
======

类Go语言消息通道 - 现代C++实现（C++11）
A Golang-like channels implementation based on C++11

## Unbuffered Channels

When sending data into unbuffered channel, a sender will block until a receiver receives the data. Likewise, a receiver will block until a sender is ready.

The program below will print 10 "ping"s.

```c
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include "../src/chan.h"
using namespace std;
using namespace chan11;

void ping(Chan<string> &ch)
{
	std::this_thread::sleep_for(std::chrono::seconds(3));
	for (int i = 0; i !=10; ++i)
		ch.send("ping");
	ch.close();
	if (ch.send("ping") == epipe)
	{
		cout << "chan closed, can't send" << endl;
	}
}

int main()
{
	Chan<string> chan = make_chan<string>();

	cout << "starting ping" << endl;
	std::thread ping_thread(ping, std::ref(chan));
	ping_thread.detach();

	string res;
	while (chan.recv(res) != epipe)
		cout << "received: " << res << endl;

	return 0;
}
```

## Buffered Channels

Buffered channels have a limited size, and we can send a limited size of data into it, this will not block until channel is "full". Receiving data from channel will block when channel is empty.

This program will print three received value, when channel is closed, we can't send any more but can receive from it.

```c
#include <iostream>
#include "../src/chan.h"
using namespace std;
using namespace chan11;

int main()
{
	Chan<int> ch = make_chan<int>(2);
	ch.send(1);
	ch.send(2);

	int res;
	ch.recv(res);
	cout << "first element: " << res << endl;
	ch.recv(res);
	cout << "second element: " << res << endl;

	ch.send(3);
	ch.close();
	ch.recv(res);
	cout << "third element: " << res << endl;
	if (ch.send(4) == epipe)
	{
		cout << "chan closed, can't send" << endl;
	}

	return 0;
}
```

## Closing a channel

When channel is closed, we can't open it any more, nor can we send data into it again, if there's data left in it, we can get the data from it until it is empty, when empty, a recv call will get a return value named epipe

```c
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
```

## Select call

A chan_select() call can be used on a set of channels, whether they are for writing or reading. If a receiving channel will be put in rCase, while a sending channel will be put in sCase. When chan_select() is called in block style, it will block until one of the channels in rCase/sCase is ready for receiving/sending, then this function will return a index indicating the channel, and we can call rCase.get() to get the received value. When called in unblock style, chan_select() will return -1 immediately if there's no channel that is ready.

```c
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
```

chan_select() will randomly return a channel when several channels are ready.

```c
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>
#include "../src/chan.h"
using namespace std;
using namespace chan11;

void rolling(vector<Chan<bool>> &channels)
{
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
```



