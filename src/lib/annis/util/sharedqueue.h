#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <limits>

namespace annis
{
  /**
   * This is a thread-safe queue that has a blocking pop() function.
   *
   * It is possible to shutdown a queue. If a queue is shutdown, not new entries
   * can be added and as soon as the queue is empty the pop() funtion will return immediatly instead of waiting forever.
   * A shutdown can't be undone.
   */
  template<typename T>
  class SharedQueue
  {
  public:

    SharedQueue()
    : isShutdown(false)
    {

    }

    /**
     * @brief Retrieve an item from the queue. This will block until an item is available. If the queue is empty
     * and shut-down it will return immediatly with "false" as a result.
     * @param item
     * @return "true" if an item was retrieved from the queue, false if not.
     */
    bool pop(T& item)
    {
      std::unique_lock<std::mutex> lock(queueMutex);

      addedCondition.wait(lock, [this] {return this->isShutdown || !this->queue.empty();});
      if(isShutdown && queue.empty())
      {
        // queue is empty and since it is shut down no new entries will be added.
        return false;
      }

      item.swap(queue.front());
      queue.pop();

      lock.unlock();

      return true;
    }

    void push(T&& item)
    {
      std::unique_lock<std::mutex> lock(queueMutex);

      if(isShutdown)
      {
        return;
      }

      queue.emplace(item);
      lock.unlock();
      addedCondition.notify_one();

    }

    void shutdown()
    {
      std::unique_lock<std::mutex> lock(queueMutex);
      if(!isShutdown)
      {
        isShutdown = true;

        lock.unlock();

        addedCondition.notify_all();
      }
    }


  private:

    bool isShutdown;

    std::queue<T> queue;

    std::mutex queueMutex;
    std::condition_variable addedCondition;

  };
}