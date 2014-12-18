#include <exception>
#include "chan.h"
using namespace chan11;


BufferedChan::BufferedChan(size_t cap) :
		capacity_(cap)
{
	if (capacity_ > 0)
		queue_ = std::make_shared<std::deque<int>>();
	else
		throw std::runtime_error("capacity not greater than zero");
}

int BufferedChan::recv()
{
	std::unique_lock u_lock(mtx_);

	while (queue_->empty())
	{
		read_cond_.wait(u_lock);
	}

	int element = queue_->front();
	queue_->pop_front();

	u_lock.unlock();
	write_cond_.notify_one();
	return element;
}

void BufferedChan::send(int e)
{
	std::unique_lock u_lock(mtx_);

	while (queue_->size() == capacity_)
	{
		write_cond_.wait(u_lock);
	}

	queue_->push_back(e);

}

int UnbufferedChan::recv()
{
	std::unique_lock u_lock(mtx_);
	while (!avail_)
	{
		read_cond_.wait(u_lock);
	}

	int element = data_;
	avail_ = false;

	u_lock.unlock();
	write_cond_.notify_one();

	return element;
}

void UnbufferedChan::send(int e)
{
	std::unique_lock u_lock(mtx_);

	while (avail_)
	{
		write_cond_.wait(u_lock);
	}

	data_ = e;
	avail_ = true;

	u_lock.unlock();
	read_cond_.notify_one();
}

Chan::Chan(int cap):
		chan_(std::make_shared<BufferedChan>(cap))
{}

Chan::Chan():
		chan_(std::make_shared<UnbufferedChan>())
{}

int Chan::recv()
{
	return chan_->recv();
}

void Chan::send(int e)
{
	chan_->send(e);
}
