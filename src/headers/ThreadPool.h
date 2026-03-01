#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

struct Task
{
    int priority;
    std::function<void()> func;

    bool operator<(const Task &other) const
    {
        return priority < other.priority;
    }
};

class ThreadPool
{
public:
    ThreadPool(size_t threads) : stop(false)
    {
        for (size_t i = 0; i < threads; ++i)
            workers.emplace_back([this]
                                 {
                for (;;) {
                    Task task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty()) return;
                        task = std::move(const_cast<Task&>(this->tasks.top()));
                        this->tasks.pop();
                    }
                    task.func();
                } });
    }

    void enqueue(int priority, std::function<void()> func)
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.push({priority, func});
        }
        condition.notify_one();
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers)
            worker.join();
    }

private:
    std::vector<std::thread> workers;
    std::priority_queue<Task> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};