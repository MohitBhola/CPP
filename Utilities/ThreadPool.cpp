#include <vector>
#include <deque>
#include <thread>
#include <future>
#include <mutex>
#include <iostream>
#include <condition_variable>
#include <functional>
#include <memory>

using namespace std;

class ThreadPool
{
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
        
    private:
    
        std::packaged_task<RetType()> func;
        
    public:
    
        AnyJob(std::packaged_task<RetType()> func) : func(std::move(func)) {}
        
        virtual void execute() 
        {
            func();
        }
    }; 
    
    mutex mMutex{};
    condition_variable mConditionVariable{};
    vector<thread> mThreadPool{};
    deque<shared_ptr<Job>> mTasks{};
    
public:

    ThreadPool(int numThreads = std::thread::hardware_concurrency())
    {
        cout << "hardware_concurrency = " << numThreads << '\n';
        
        for (int i = 0; i < numThreads; ++i)
        {
            mThreadPool.emplace_back(
                thread(
                    [this]()
                    {
                        while (true)
                        {
                            shared_ptr<Job> pTask{};
                            {
                                unique_lock<mutex> lk(mMutex);
                                mConditionVariable.wait(lk, [&](){return !mTasks.empty();});
                                pTask = mTasks.back(); mTasks.pop_back();
                                
                                if (pTask == nullptr)
                                {
                                    mTasks.push_back(nullptr);
                                    return;
                                }
                            }
                            
                            pTask->execute();
                    }
            }));
        }
    }
    
    ~ThreadPool()
    {
        {
            unique_lock<mutex> lk(mMutex);
            mTasks.push_back(nullptr);
            mConditionVariable.notify_all();
        }
        
        for (thread& t : mThreadPool)
        {
            t.join();
        }
    }
     
    template<
        typename F,
        typename... Args>
    future<result_of_t<F(Args...)>> enqueue_task(F&& f, Args&&... args) 
    { 
        using return_type = result_of_t<F(Args...)>;
        
        packaged_task<return_type()> p(move(bind(forward<F>(f), forward<Args>(args)...)));
        future<return_type> fut = p.get_future();
        
        {
            unique_lock<mutex> lk(mMutex);   
            mTasks.emplace_back(make_shared<AnyJob<return_type>>(move(p)));
            mConditionVariable.notify_all();
        }
        
        return fut;
    }
    
    void cancel_pending()
    {
        unique_lock<mutex> lk(mMutex);
        mTasks.clear();
        mConditionVariable.notify_all();
    }
};

void add(int a, int b, promise<int> p)
{
    p.set_value(a+b);
}

int multiply(int a, int b)
{
    return a*b;   
}

int main()
{
    ThreadPool tp{};
    
    auto f = tp.enqueue_task(multiply, 1, 2);
    cout << f.get() << '\n';
    
    return 0;
}
