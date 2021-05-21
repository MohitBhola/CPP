#include <functional>
#include <memory>
#include <iostream>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <future>

using namespace std;

class Job
{
    
public:

    ~Job() noexcept = default;
    virtual void execute() = 0;
};

template <
    typename RetType>
class AnyJob : public Job
{
    packaged_task<RetType()> mTask{};
    
public:

    AnyJob(packaged_task<RetType()> t) : mTask(move(t)) {}
    
    virtual void execute()
    {
        mTask();
    }
};

class BecomeActiveObject
{
    double val{};
    mutex mMutex{};
    condition_variable mCV{};
    deque<shared_ptr<Job>> q{};
    
    unique_ptr<thread> mRunnable{};
    
public:

    BecomeActiveObject()
    {
        mRunnable = make_unique<thread>(
            [this]()
            {
                while (true)
                { 
                    shared_ptr<Job> pJob{};
                    {
                        unique_lock<mutex> lk(mMutex);
                        mCV.wait(lk, [this](){return !q.empty();});
                        pJob = move(q.back()); q.pop_back();
                    }
                    
                    if (!pJob)
                    {
                        return;
                    }
                    
                    pJob->execute();
                }
            }
        );
    }
    
    ~BecomeActiveObject() 
    {
        {
            unique_lock<mutex> lk(mMutex);
            q.push_front(nullptr);
        }
        
        mRunnable->join();
    }
        
    auto func1()
    {
        auto l = [this]()
        {
            val = 1.1;
            cout << "func1" << '\n';
        };
        
        return enqueue_work(l);
    }
    
    auto func2()
    {
        auto l = [this]()
        {
            val = 2.2;
            cout << "func2" << '\n';
        };
        
        return enqueue_work(l);
    }

private:

    template<
        typename F,
        typename... Args>
    future<result_of_t<F(Args...)>> enqueue_work(F&& f, Args&&... args)
    {
        using RetType = result_of_t<F(Args...)>;
        
        packaged_task<RetType()> p(bind(forward<F>(f), forward<Args>(args)...));
        future<RetType> fut = p.get_future();
        
        {
            unique_lock<mutex> lk(mMutex);
            q.push_front(make_shared<AnyJob<RetType>>(move(p)));
            mCV.notify_all();
        }
        
        return fut;
    }
};

int main()
{
    BecomeActiveObject ao{};
    ao.func1();
    ao.func2();
    
    return 0;
}
