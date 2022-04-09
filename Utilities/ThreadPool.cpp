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
    // a generic job
    class Job
    {
   
    public:
    
        ~Job() noexcept = default;
        virtual void execute() = 0;
    };
    
    // a specific job with a concrete return type
    template <
        typename RetType>
    class AnyJob : public Job 
    {
        
    private:
    
        // the callable under the hood of the packaged_task shall 
        // invoke the real callable with all required arguments
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
                    // all threads in the thread pool continuously execute this method
                    [this] ()
                    {
                        while (true)
                        {
                            shared_ptr<Job> pTask{};
                            {
                                unique_lock<mutex> lk(mMutex);
                                mConditionVariable.wait(lk, [&](){return !mTasks.empty();});
                                pTask = mTasks.back(); mTasks.pop_back();
                                
                                // if we fetch a null job, it means we are shutting down
                                // add a null job in the job queue that would subsequently be
                                // fetched by some other thread in the pool
                                // also, issue a wake up call for sleeping threads
                                if (pTask == nullptr)
                                {
                                    mTasks.push_back(nullptr);
                                    mConditionVariable.notify_all();
        
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
            
            // enqueue a null job
            // the thread that picks up this job shall 
            // enqueue a null job for the next and so on
            // ultimately, all threads shall exit
            mTasks.push_back(nullptr);
            mConditionVariable.notify_one();
        }
        
        for (thread& t : mThreadPool)
        {
            // wait for all threads to have fetched the null job and exit
            t.join();
        }
    }
     
    template<
        typename F,
        typename... Args>
    future<result_of_t<F(Args...)>> enqueue_task(F&& f, Args&&... args) 
    { 
        // we have a callable *F* and a variadic parameter *args*
        // we shall create a packaged_task which shall wrap around a callable that would call F with the variadic parameter *args*
        // that callable shall be produced via a call to bind with *F* and *args* as the arguments
        // the packaged_task template argument is <RetType()>, where RetType is the type of the result produced
        // by the callable F when invoked with the variadic parameter *args*
        using RetType = result_of_t<F(Args...)>;
        
        packaged_task<RetType()> p(move(bind(forward<F>(f), forward<Args>(args)...)));
        future<RetType> fut = p.get_future();
        
        {
            unique_lock<mutex> lk(mMutex);   
            mTasks.emplace_back(make_shared<AnyJob<RetType>>(move(p)));
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
