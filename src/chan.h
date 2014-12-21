#ifndef CHAN_H
#define CHAN_H
#include <deque>
#include <memory>
#include <condition_variable>
#include <mutex>

namespace chan11
{

// TODO: channel can be closed
enum errcode_t
{
	esucc = 0,
	epipe = -(esucc + 1)
};

class BaseChan
{
	friend class Chan;
public:
	virtual ~BaseChan() = default;
private:
	virtual errcode_t recv(int&) = 0;
	virtual errcode_t send(int) = 0;
	virtual bool buffered() const = 0;
protected:
	errcode_t close();
	bool closed();
	bool closed_ = false;
	std::mutex mtx_;
	std::condition_variable write_cond_;
	std::condition_variable read_cond_;
};

// this class serves as the interface and hide the hierarchy
// of the relationship above
class Chan
{
public:
	Chan(int);
	Chan();
	errcode_t recv(int&);
	errcode_t send(int);
private:
	std::shared_ptr<BaseChan> chan_;
};

class BufferedChan : public BaseChan
{
	friend class Chan;
public:
	BufferedChan(size_t cap);
private:
	virtual errcode_t recv(int&) override;
	virtual errcode_t send(int) override;
	virtual bool buffered() const override { return true; }
	size_t size();
	size_t capacity();
	size_t capacity_;
	std::shared_ptr<std::deque<int>> queue_;
};

class UnbufferedChan : public BaseChan
{
	friend class Chan;
public:
	UnbufferedChan() = default;
private:
	virtual errcode_t recv(int&) override;
	virtual errcode_t send(int) override;
	virtual bool buffered() const override { return false; }
	bool avail_ = false;
	std::shared_ptr<int> data_;
};

Chan make_chan(int);
Chan make_chan();
} // chan11
#endif//CHAN_H
