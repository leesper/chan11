#include <iostream>
#include "../src/chan.h"
using namespace std;
using namespace chan11;

int main()
{
	Chan ch = make_chan(2);
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
