#include <iostream>
#include "../src/chan.h"
using namespace std;
using namespace chan11;

int main()
{
	Chan ch = make_chan(2);
	ch.send(1);
	ch.send(2);

	int res1, res2;
	ch.recv(res1);
	ch.recv(res2);
	cout << "first element: " << res1 << endl;
	cout << "second element: " << res2 << endl;

	return 0;
}
