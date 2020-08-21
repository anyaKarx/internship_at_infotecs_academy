#pragma once
#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

#include <queue>

template<class T>
class Buffer 
{
    std::queue<T> _buffer;
    mutable boost::mutex _buffer_change;
    boost::condition_variable _condition_variable;
        
public:
    Buffer() = default;
    
    template<typename InputValueType>
    void push(InputValueType&& data)
    {
        boost::unique_lock<decltype(_buffer_change)> lock(_buffer_change);
        _buffer.push(std::forward<InputValueType>(data));
        _condition_variable.notify_one();
    }

    bool empty() const
    {
        boost::shared_lock<decltype(_buffer_change)> lock(_buffer_change);
        return _buffer.empty();
    }

    T pop()
    {
        boost::unique_lock<decltype(_buffer_change)> lock(_buffer_change);
        _condition_variable.wait(lock, [&buffer = _buffer]() {return !buffer.empty(); });
        T element = _buffer.front();
        _buffer.pop();
        return element;
    }

    void clear()
    {
        boost::unique_lock<decltype(_buffer_change)> lock(_buffer_change);
        std::queue<T>{}.swap(_buffer);
    }
};
