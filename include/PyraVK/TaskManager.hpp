#pragma once
#include <PyraVK/PyraBase.h>
#include <functional>
#include <future>
#include <mutex>
#include <queue>


namespace pyra
{

    template<class Rt, class...Args>
    struct Task
    {
        std::promise<Rt> m_result_promise;
        std::function<Rt(Args...)> m_task_func;
    };

    class TaskManager : public PyraBase
    {
        std::queue<std::function<void()>> m_tasks;
        std::mutex m_queue_mutex;
        std::condition_variable m_update_tracker;

        std::vector<std::thread> m_worker_threads;
        void worker_thread();
        bool m_exit = false;
    public :
        TaskManager();
        ~TaskManager();

        template<class F, class...Args>
        std::future<std::invoke_result_t<F,Args...>> PushTask(F&& func,Args&&...args);
    };

    inline TaskManager::TaskManager()
    {
        uint32_t worker_thread_count = std::max(1u,std::thread::hardware_concurrency() - 3);
        m_worker_threads.reserve(worker_thread_count);
        for (int i = 0; i < worker_thread_count; ++i)
        {
            m_worker_threads.emplace_back(&TaskManager::worker_thread,this);
        }
    }

    inline TaskManager::~TaskManager()
    {
        m_exit = true;
        m_update_tracker.notify_all();

        for (auto& worker_thread : m_worker_threads)
        {
            worker_thread.join();
        }
    }

    inline void TaskManager::worker_thread()
    {
        while (true)
        {
            std::function<void()> task;

            {
                std::unique_lock lock(m_queue_mutex);
                m_update_tracker.wait(lock,[this]() -> bool {return m_exit || !m_tasks.empty();});

                if (m_exit) return;
                task = std::move(m_tasks.front());
                m_tasks.pop();
            }

            task();
        }
    }

    template <class F, class ... Args>
    std::future<std::invoke_result_t<F,Args...>> TaskManager::PushTask(F&& func, Args&&... args)
    {
        using return_type = std::invoke_result_t<F,Args...>;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            [f = std::forward<F>(func), ...args_ = std::forward<Args>(args)]() -> return_type { return f(args_...); });

        auto wrapper = [task]()
        {
            (*task)();
        };

        std::lock_guard lock(m_queue_mutex);
        m_tasks.push(std::move(wrapper));
        m_update_tracker.notify_one();
        return task->get_future();
    }



}
