#include <exception>
#include "chan.h"

namespace chan11 {

errcode_t BaseChan::close()
{
	std::unique_lock<std::mutex> u_lock(mtx_);

	if (closed_)
		return epipe;

	closed_ = true;
	u_lock.unlock();

	read_cond_.notify_all();
	write_cond_.notify_all();

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

bool BufferedChan::recv_ready()
{
	std::unique_lock<std::mutex> u_lock(mtx_);
	bool isOK = queue_->size() > 0;
	return isOK;
}

bool BufferedChan::send_ready()
{
	std::unique_lock<std::mutex> u_lock(mtx_);
	// only if queue_ has room and BufferedChan is not closed
	bool isOK = queue_->size() < capacity_ && !closed_;
	return isOK;
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

bool UnbufferedChan::recv_ready()
{
	std::unique_lock<std::mutex> u_lock(mtx_);
	bool isOK = avail_;
	return isOK;
}

bool UnbufferedChan::send_ready()
{
	std::unique_lock<std::mutex> u_lock(mtx_);
	bool isOK = !avail_;
	return isOK;
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

errcode_t Chan::close()
{
	return chan_->close();
}

Chan make_chan(int cap)
{
	return Chan(cap);
}

Chan make_chan()
{
	return Chan();
}

bool rCase::ready()
{
	return ch_.recv_ready();
}

errcode_t rCase::exec()
{
	return ch_.recv(val_);
}

int rCase::get()
{
	return val_;
}

bool sCase::ready()
{
	return ch_.send_ready();
}

errcode_t sCase::exec()
{
	return ch_.send(val_);
}

int sCase::get()
{
	throw std::runtime_error("calling get on a sCase");
}

int chan_select(std::vector<std::shared_ptr<Case>> &cases, bool blocked)
{
	int index = -1;

	if (cases.empty()) return index;

	if (!blocked)
	{
		// TODO: random in range [0..cases.size()-1]
		for (auto i = 0; i != cases.size(); ++i)
		{
			if (cases[i]->ready())
			{
				index = i;
				cases[i]->exec();
				return index;
			}
		}
	}
	else
	{
		while (true)
		{
			// TODO
			for (auto i = 0; i != cases.size(); ++i)
			{
				if (cases[i]->ready())
				{
					index = i;
					cases[i]->exec();
					return index;
				}
			}
		}
	}

	return index;
}
} // namespace chan11
