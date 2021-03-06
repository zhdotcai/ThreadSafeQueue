// From the personal library of Paul Ilardi (http://github.com/CodePi).
// Free to use or modify.

#pragma once

#include <queue>
#include <stack>
#include <mutex>
#include <condition_variable>

namespace codepi{

template <class T>
class ThreadSafeQueue {
public:

  ThreadSafeQueue(bool useStack = false) : useStack(useStack) {}

  // enqueue - supports move, copies only if needed. e.g. q.enqueue(move(obj));
  void enqueue(T t){
    std::lock_guard<std::mutex> lock(m);
    if(useStack) s.push(std::move(t));
    else         q.push(std::move(t));
    c.notify_one();
  }

  // simple dequeue
  T dequeue(){
    std::unique_lock<std::mutex> lock(m);
    while(empty()) c.wait(lock);
    return pop();
  }

  // dequeue with timeout in seconds
  bool dequeue(double timeout_sec, T& rVal){
    std::unique_lock<std::mutex> lock(m);
    bool isTimeout=false;

    // wait for timeout or value available
    auto maxTime = std::chrono::milliseconds(int(timeout_sec*1000));
    if(c.wait_for(lock, maxTime, [&](){return !this->empty();} )){
      rVal = pop();
      return true;
    } else {
      return false;
    }
  }

  size_t size() const { return useStack? s.size()  : q.size(); }
  bool  empty() const { return useStack? s.empty() : q.empty(); }
  void  clear() {
    std::lock_guard<std::mutex> lock(m);
    // stack and queue have no clear method!! Maybe switch to deque
    while(!empty()) pop();
  }

private:
  std::queue<T> q;
  std::stack<T> s;
  const bool useStack;
  mutable std::mutex m;
  std::condition_variable c;
  
  T pop(){
    if(useStack){
      T val = std::move(s.top());
      s.pop();
      return val;
    }else{
      T val = std::move(q.front());
      q.pop();
      return val;
    }
  }
    
};

} // end namespace codepi
