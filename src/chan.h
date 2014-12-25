#ifndef CHAN_H
#define CHAN_H
#include <deque>
#include <vector>
#include <memory>
#include <condition_variable>
#include <mutex>

namespace chan11
{

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

// this class serves as the interface and hide the hierarchy
// of the relationship above
class Chan
{
public:
	Chan(int);
	Chan();
	errcode_t recv(int&);
	errcode_t send(int);
	errcode_t close();
	bool recv_ready() { return chan_->recv_ready(); }
	bool send_ready() { return chan_->send_ready(); }
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
	virtual bool recv_ready() override;
	virtual bool send_ready() override;
	size_t size();
	size_t capacity();
	size_t capacity_;
	std::shared_ptr<std::deque<int>> queue_;
};

class UnbufferedChan : public BaseChan
{
	friend class Chan;
public:
	UnbufferedChan(): data_(std::make_shared<int>()) {}
private:
	virtual errcode_t recv(int&) override;
	virtual errcode_t send(int) override;
	virtual bool buffered() const override { return false; }
	virtual bool recv_ready() override;
	virtual bool send_ready() override;
	bool avail_ = false;
	std::shared_ptr<int> data_;
};

class Case
{
public:
	Case(Chan ch): ch_(ch) {}
	virtual ~Case() {}
	virtual bool ready() = 0;
	virtual errcode_t exec() = 0;
	virtual int get() = 0;
protected:
	Chan ch_;
};

class rCase: public Case
{
public:
	rCase(Chan ch): Case(ch) {}
	virtual bool ready() override;
	virtual errcode_t exec() override;
	virtual int get() override;
private:
	int val_ = 0;
};

class sCase: public Case
{
public:
	sCase(Chan ch, int val): Case(ch), val_(val) {}
	virtual bool ready() override;
	virtual errcode_t exec() override;
	virtual int get() override;
private:
	int val_;
};

Chan make_chan(int);
Chan make_chan();
int chan_select(std::vector<std::shared_ptr<Case>>&, bool = false);
// TODO: make Chan a class template
} // chan11
#endif//CHAN_H
