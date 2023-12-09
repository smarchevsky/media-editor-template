#pragma once

#include <future>
#include <mutex>
#include <thread>

#include <deque>
#include <vector>

// ThreadPool

// Created to write images to files in different threads
// void queue(F&& f) - adds task
// if (tasks.size() >= (numCores - 1)) - locks the thread, where you are trying to add task
// when available free core - unlocks thread to add task
// ~ThreadPool calls "finish", it waits, until remain tasks finish
// If "finish(force == true)" remaining tasks will clear, wait for currently executed task only

struct ThreadPool {
    std::mutex m;
    std::condition_variable executionCondition, m_mainThreadCondition;
    std::deque<std::packaged_task<void()>> m_tasks;
    std::vector<std::future<void>> m_workers;

    int getMaxThreads() { return std::max(std::thread::hardware_concurrency() - 1u, 1u); }
    int getAvailableThreadNum() { return getMaxThreads() - m_tasks.size(); }

    ThreadPool()
    {
        for (std::size_t i = 0; i < getMaxThreads(); ++i) {
            m_workers.push_back(std::async(std::launch::async, [this] { thread_task(); }));
        }
    }

    template <class F, class R = std::result_of_t<F&()>>
    std::future<R> queue(F&& f)
    {
        std::packaged_task<R()> p(std::forward<F>(f));
        auto r = p.get_future();
        {
            std::unique_lock<std::mutex> l(m);

            m_mainThreadCondition.wait(l, [&] { // remove this wait to add endless amount of tasks
                bool run = getAvailableThreadNum() > 0;
                return run;
            });
            m_tasks.emplace_back(std::move(p));
        }
        executionCondition.notify_one();
        return r;
    }

    void abort()
    {
        cancel_pending();
        finish();
    }

    void cancel_pending()
    {
        std::unique_lock<std::mutex> l(m);
        m_tasks.clear();
    }

    void finish()
    {
        {
            std::unique_lock<std::mutex> l(m);
            for (auto&& unused : m_workers) {
                m_tasks.push_back({});
            }
        }
        executionCondition.notify_all();
        m_workers.clear();
    }
    ~ThreadPool()
    {
        finish();
    }

private:
    void thread_task()
    {
        while (true) {
            std::packaged_task<void()> f;
            {
                std::unique_lock<std::mutex> l(m);
                if (m_tasks.empty()) {
                    executionCondition.wait(l, [&] { return !m_tasks.empty(); });
                }
                m_mainThreadCondition.notify_one();

                f = std::move(m_tasks.front());
                m_tasks.pop_front();
            }

            if (!f.valid())
                return;
            f();
        }
    }
};
