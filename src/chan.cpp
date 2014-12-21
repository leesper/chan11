#include <exception>
#include "chan.h"

namespace chan11 {

errcode_t BaseChan::close()
{
	std::lock_guard<std::mutex> lg(mtx_);

	if (closed_)
		return epipe;

	closed_ = true;
	return esucc;
}

bool BaseChan::closed()
{
	std::lock_guard<std::mutex> lg(mtx_);
	bool is_closed = closed_;
	return is_closed;
}

BufferedChan::BufferedChan(size_t cap) :
		capacity_(cap)
{
	if (capacity_ > 0)
		queue_ = std::make_shared<std::deque<int>>();
	else
		throw std::runtime_error("capacity not greater than zero");
}

errcode_t BufferedChan::recv(int &e)
{
	std::unique_lock<std::mutex> u_lock(mtx_);

	while (queue_->empty())
	{
		if (closed_)
		{
			return epipe;
		}

		read_cond_.wait(u_lock);
	}

	e = queue_->front();
	queue_->pop_front();

	u_lock.unlock();
	write_cond_.notify_one();
	return esucc;
}

errcode_t BufferedChan::send(int e)
{
	std::unique_lock<std::mutex> u_lock(mtx_);

	while (queue_->size() >= capacity_)
	{
		write_cond_.wait(u_lock);
	}

	if (closed_)
		return epipe;

	queue_->push_back(e);
	return esucc;
}

size_t BufferedChan::size()
{
	std::unique_lock<std::mutex> u_lock(mtx_);
	size_t sz = queue_->size();
	return sz;
}

size_t BufferedChan::capacity()
{
	std::unique_lock<std::mutex> u_lock(mtx_);
	size_t cap = capacity_;
	return cap;
}

errcode_t UnbufferedChan::recv(int &e)
{
	std::unique_lock<std::mutex> u_lock(mtx_);

	while (!avail_)
	{
		if (closed_)
			return epipe;

		read_cond_.wait(u_lock);
	}

	e = *data_;
	avail_ = false;

	u_lock.unlock();
	write_cond_.notify_one();

	return esucc;
}

errcode_t UnbufferedChan::send(int e)
{
	std::unique_lock<std::mutex> u_lock(mtx_);

	while (avail_)
	{
		write_cond_.wait(u_lock);
	}

	if (closed_)
		return epipe;

	*data_ = e;
	avail_ = true;

	u_lock.unlock();
	read_cond_.notify_one();
	return esucc;
}

Chan::Chan(int cap):
		chan_(std::make_shared<BufferedChan>(cap))
{}

Chan::Chan():
		chan_(std::make_shared<UnbufferedChan>())
{}

errcode_t Chan::recv(int &e)
{
	return chan_->recv(e);
}

errcode_t Chan::send(int e)
{
	return chan_->send(e);
}

Chan make_chan(int cap)
{
	return Chan(cap);
}

Chan make_chan()
{
	return Chan();
}

} // namespace chan11
