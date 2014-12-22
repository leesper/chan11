#include <iostream>
#include <thread>
#include <chrono>
#include "../src/chan.h"
using namespace std;
using namespace chan11;

void ping(Chan &ch)
{
	std::this_thread::sleep_for(std::chrono::seconds(3));
	for (int i = 0; i !=10; ++i)
		ch.send(i);
	ch.close();
	if (ch.send(20) == epipe)
	{
		cout << "chan closed, can't send" << endl;
	}
}

int main()
{
	Chan chan = make_chan();

	cout << "starting ping" << endl;
	std::thread ping_thread(ping, std::ref(chan));
	ping_thread.detach();

	int res;
	while (chan.recv(res) != epipe)
		cout << "received: " << res << endl;

	return 0;
}
