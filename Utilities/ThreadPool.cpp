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
        // we have a callable F and variadic arguments args
        // the packaged_task template argument is <RetType()>
        // this means that the return type of the callable under the hood of the packaged_task is RetType
        // but that callable shall take no arguments at runtime
        // this would be made feasible by binding the arguments to the callable F, thus producing the callable under the hood of the packaged_task
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
