#ifndef CHAN_H
#define CHAN_H
#include <deque>
#include <vector>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <random>
#include <algorithm>

#include <iostream>

namespace chan11
{

enum errcode_t
{
	esucc = 0,
	epipe = -(esucc + 1)
};

template <typename> class Chan;
// the abstract base class for channel
template <typename T> class BaseChan
{
	friend class Chan<T>;
public:
	virtual ~BaseChan() = default;
private:
	virtual errcode_t recv(T&) = 0;
	virtual errcode_t send(T) = 0;
	virtual bool buffered() const = 0;
	virtual bool recv_ready() = 0;
	virtual bool send_ready() = 0;
protected:
	errcode_t close();
	bool closed();
	bool closed_ = false;
	std::mutex mtx_;
	std::condition_variable write_cond_;
	std::condition_variable read_cond_;
};

template <typename T> errcode_t BaseChan<T>::close()
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

template <typename T> bool BaseChan<T>::closed()
{
	std::lock_guard<std::mutex> lg(mtx_);
	bool is_closed = closed_;
	return is_closed;
}

// buffered channel
template <typename T> class BufferedChan : public BaseChan<T>
{
	friend class Chan<T>;
public:
	BufferedChan(size_t cap);
	virtual ~BufferedChan() {}
private:
	virtual errcode_t recv(T&) override;
	virtual errcode_t send(T) override;
	virtual bool buffered() const override { return true; }
	virtual bool recv_ready() override;
	virtual bool send_ready() override;
	size_t size();
	size_t capacity();
	size_t capacity_;
	std::shared_ptr<std::deque<T>> queue_;
};

template <typename T>
BufferedChan<T>::BufferedChan(size_t cap) :
		capacity_(cap)
{
	if (capacity_ > 0)
		queue_ = std::make_shared<std::deque<T>>();
	else
		throw std::runtime_error("capacity not greater than zero");
}

template <typename T> errcode_t BufferedChan<T>::recv(T &e)
{
	std::unique_lock<std::mutex> u_lock(this->mtx_);

	while (queue_->empty())
	{
		if (this->closed_)
		{
			return epipe;
		}

		this->read_cond_.wait(u_lock);
	}

	e = queue_->front();
	queue_->pop_front();

	u_lock.unlock();
	this->write_cond_.notify_one();
	return esucc;
}

template <typename T>
errcode_t BufferedChan<T>::send(T e)
{
	std::unique_lock<std::mutex> u_lock(this->mtx_);

	while (queue_->size() >= capacity_)
	{
		this->write_cond_.wait(u_lock);
	}

	if (this->closed_)
		return epipe;

	queue_->push_back(e);
	return esucc;
}

template <typename T>
bool BufferedChan<T>::recv_ready()
{
	std::unique_lock<std::mutex> u_lock(this->mtx_);
	bool isOK = queue_->size() > 0;
	return isOK;
}

template <typename T>
bool BufferedChan<T>::send_ready()
{
	std::unique_lock<std::mutex> u_lock(this->mtx_);
	// only if queue_ has room and BufferedChan is not closed
	bool isOK = queue_->size() < capacity_ && !this->closed_;
	return isOK;
}

template <typename T>
size_t BufferedChan<T>::size()
{
	std::unique_lock<std::mutex> u_lock(this->mtx_);
	size_t sz = queue_->size();
	return sz;
}

template <typename T>
size_t BufferedChan<T>::capacity()
{
	std::unique_lock<std::mutex> u_lock(this->mtx_);
	size_t cap = capacity_;
	return cap;
}

// unbuffered channel
template <typename T> class UnbufferedChan : public BaseChan<T>
{
	friend class Chan<T>;
public:
	UnbufferedChan(): data_(std::make_shared<T>()) {}
	virtual ~UnbufferedChan() {}
private:
	virtual errcode_t recv(T&) override;
	virtual errcode_t send(T) override;
	virtual bool buffered() const override { return false; }
	virtual bool recv_ready() override;
	virtual bool send_ready() override;
	bool avail_ = false;
	std::shared_ptr<T> data_;
};

template <typename T>
errcode_t UnbufferedChan<T>::recv(T &e)
{
	std::unique_lock<std::mutex> u_lock(this->mtx_);

	while (!avail_)
	{
		if (this->closed_)
			return epipe;

		this->read_cond_.wait(u_lock);
	}

	e = *data_;
	avail_ = false;

	u_lock.unlock();
	this->write_cond_.notify_one();

	return esucc;
}

template <typename T>
errcode_t UnbufferedChan<T>::send(T e)
{
	std::unique_lock<std::mutex> u_lock(this->mtx_);

	while (avail_)
	{
		this->write_cond_.wait(u_lock);
	}

	if (this->closed_)
		return epipe;

	*data_ = e;
	avail_ = true;

	u_lock.unlock();
	this->read_cond_.notify_one();
	return esucc;
}

template <typename T>
bool UnbufferedChan<T>::recv_ready()
{
	std::unique_lock<std::mutex> u_lock(this->mtx_);
	bool isOK = avail_;
	return isOK;
}

template <typename T>
bool UnbufferedChan<T>::send_ready()
{
	std::unique_lock<std::mutex> u_lock(this->mtx_);
	bool isOK = !avail_;
	return isOK;
}

// this class serves as the interface and hide the hierarchy
// of the relationship of BaseChan, BufferedChan and UnbufferedChan
template <typename T> class Chan
{
public:
	Chan(size_t);
	Chan();
	errcode_t recv(T&);
	errcode_t send(T);
	errcode_t close();
	bool recv_ready() { return chan_->recv_ready(); }
	bool send_ready() { return chan_->send_ready(); }
private:
	std::shared_ptr<BaseChan<T>> chan_;
};

template <typename T>
Chan<T>::Chan(size_t cap):
		chan_(std::make_shared<BufferedChan<T>>(cap))
{}

template <typename T>
Chan<T>::Chan():
		chan_(std::make_shared<UnbufferedChan<T>>())
{}

template <typename T>
errcode_t Chan<T>::recv(T &e)
{
	return chan_->recv(e);
}

template <typename T>
errcode_t Chan<T>::send(T e)
{
	return chan_->send(e);
}

template <typename T>
errcode_t Chan<T>::close()
{
	return chan_->close();
}

// base class for function chan_select()
template <typename T> class Case
{
public:
	Case(Chan<T> ch): ch_(ch) {}
	virtual ~Case() {}
	virtual bool ready() = 0;
	virtual errcode_t exec() = 0;
	virtual T get() = 0;
protected:
	Chan<T> ch_;
};

// read Case, means we can get data from it
template <typename T> class rCase: public Case<T>
{
public:
	rCase(Chan<T> ch): Case<T>(ch) {}
	virtual ~rCase() {}
	virtual bool ready() override;
	virtual errcode_t exec() override;
	virtual T get() override;
private:
	T val_ = 0;
};

template <typename T>
bool rCase<T>::ready()
{
	return this->ch_.recv_ready();
}

template <typename T>
errcode_t rCase<T>::exec()
{
	return this->ch_.recv(val_);
}

template <typename T>
T rCase<T>::get()
{
	return val_;
}

// send Case, means we can send data to it
template <typename T>
class sCase: public Case<T>
{
public:
	sCase(Chan<T> ch, T val): Case<T>(ch), val_(val) {}
	virtual ~sCase() {}
	virtual bool ready() override;
	virtual errcode_t exec() override;
	virtual T get() override;
private:
	T val_;
};

template <typename T>
bool sCase<T>::ready()
{
	return this->ch_.send_ready();
}

template <typename T>
errcode_t sCase<T>::exec()
{
	return this->ch_.send(val_);
}

template <typename T>
T sCase<T>::get()
{
	throw std::runtime_error("calling get on a sCase");
}

// construct a buffered channel
template <typename T>
Chan<T> make_chan(size_t cap)
{
	return Chan<T>(cap);
}

// construct an unbuffered channel
template <typename T>
Chan<T> make_chan()
{
	return Chan<T>();
}

template <typename T>
int chan_select(std::vector<std::shared_ptr<Case<T>>> &cases, bool blocked = true)
{
	int index = -1;

	if (cases.empty()) return index;

	auto rand_cases = cases;

	std::random_device rd;
	std::mt19937 gen(rd());

	std::shuffle(rand_cases.begin(), rand_cases.end(), gen);
	if (!blocked)
	{
		for (auto rc : rand_cases)
		{
			if (rc->ready())
			{
				auto iter = std::find(cases.begin(), cases.end(), rc);
				index = iter - cases.begin();
				rc->exec();
				return index;
			}
		}
	}
	else
	{
		while (true)
		{
			for (auto rc : rand_cases)
			{
				if (rc->ready())
				{
					auto iter = std::find(cases.begin(), cases.end(), rc);
					index = iter - cases.begin();
					rc->exec();
					return index;
				}
			}
		}
	}

	return index;
}

} // chan11
#endif//CHAN_H
