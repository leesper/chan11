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
