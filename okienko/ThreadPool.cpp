#include <vector>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <thread>
#include <functional>
#include <future>

class ThreadPool
{
public:
    // Constructor starts up a specified number of worker threads and starts them to wait for tasks
    ThreadPool(size_t threads) : stop(false)
    {
        for (size_t i = 0; i < threads; ++i)
            workers.emplace_back([this]
                                 {
            for (;;) {
                std::function<void()> task;
                {
                    // Acquire lock to get task
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    // Wait until there is a task to perform or stop signal is sent
                    this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                    // If stop signal received and there are no tasks left, exit the thread
                    if (this->stop && this->tasks.empty())
                        return;
                    // Get next task in the queue
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                // Execute the task
                task();
            } });
    }

    // Add a new task to the queue of tasks for workers to execute
    template <class F, class... Args>
    auto enqueue(F &&function, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;
        // Create a package that stores the task to be executed
        auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(function), std::forward<Args>(args)...));

        // Get the future that will hold the result of the task
        std::future<return_type> res = task->get_future();
        {
            // Acquire lock to add task to the queue
            std::unique_lock<std::mutex> lock(queue_mutex);
            // Don't allow enqueueing after stopping the pool
            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace([task]()
                          { (*task)(); });
        }
        // Notify one waiting thread that there is a new task to execute
        condition.notify_one();
        return res;
    }

    // Destructor notifies all threads to stop and waits for all of them to join
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
    // All the worker threads
    std::vector<std::thread> workers;
    // Task queue
    std::queue<std::function<void()>> tasks;
    // Synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    // Indicates that the pool is stopping
    bool stop;
};