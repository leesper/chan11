#ifndef CHAN_H
#define CHAN_H
#include <deque>
#include <memory>
#include <condition_variable>
#include <mutex>

namespace chan11
{

// TODO: channel can be closed
class BaseChan
{
	friend class Chan;
protected:
	virtual ~BaseChan();
	std::mutex mtx_;
	std::condition_variable write_cond_;
	std::condition_variable read_cond_;
private:
	virtual int recv() = 0;
	virtual void send(int) = 0;
	virtual bool buffered() const = 0;
};

class BufferedChan : public BaseChan
{
protected:
	BufferedChan() = delete;
	BufferedChan(size_t cap);
	virtual int recv() override;
	virtual void send(int) override;
	virtual bool buffered() const override { return true; }
	size_t size() const { return queue_->size(); }
	size_t capacity() const { return capacity_; }
	size_t capacity_;
protected:
	std::shared_ptr<std::deque<int>> queue_;
};

class UnbufferedChan : public BaseChan
{
protected:
	UnbufferedChan() = default;
	virtual int recv() override;
	virtual void send(int) override;
	virtual bool buffered() const override { return false; }
protected:
	bool avail_ = false;
	std::shared_ptr<int> data_;
};

// this class serves as the interface and hide the hierarchy
// of the relationship above
class Chan
{
public:
	Chan(int);
	Chan();
	int recv();
	void send(int);
private:
	std::shared_ptr<BaseChan> chan_;
};

Chan make_chan(int);
Chan make_chan();
} // chan11
#endif//CHAN_H