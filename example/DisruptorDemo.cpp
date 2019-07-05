//g++ -std=c++14 -g -rdynamic DisruptorDemo.cpp -o DisruptorDemo  -I../ -L/root/Disruptor-cpp/example/../build/Disruptor -Wl,-rpath,/root/Disruptor-cpp/example/../build/Disruptor -lDisruptor 
#include "Disruptor/Disruptor.h"
#include "Disruptor/ThreadPerTaskScheduler.h"
#include <iostream>
#include <string>
#include <cstdint>
#include <memory>
#include <cstdlib>
#include <time.h>
#include <unistd.h>

// 定义事件体
struct Event
{
    int64_t id{ 0 };
    std::string str;
};

// 继承事件处理器接口
class Worker : public Disruptor::IWorkHandler< Event >
{
public:
    explicit Worker(){}
    // 重写事件处理回调函数
    void onEvent(Event& event) override
    {
        //usleep(1000);
        _actuallyProcessed++;
	    std::cout << "work:" << event.id << ":" << std::this_thread::get_id() << ":" << _actuallyProcessed << std::endl;
    }

private:
    int32_t _actuallyProcessed{ 0 };
};
// 继承事件处理器接口
class Worker1 : public Disruptor::IEventHandler< Event >
{
public:
    explicit Worker1(){}
    // 重写事件处理回调函数
    void onEvent(Event& event, std::int64_t sequence, bool endOfBatch) override
    {
        //usleep(1000);
        _actuallyProcessed++;
	    std::cout << "work1:"  << event.id << ":" << std::this_thread::get_id() << ":" << _actuallyProcessed << std::endl;
    }

private:
    int32_t _actuallyProcessed{ 0 };
};
// 继承事件处理器接口
class Worker2 : public Disruptor::IEventHandler< Event >
{
public:
    explicit Worker2(){}
    // 重写事件处理回调函数
    void onEvent(Event& event, std::int64_t sequence, bool endOfBatch) override
    {
        //usleep(1000);
        _actuallyProcessed++;
	    std::cout << "work2:"  << event.id << ":" << std::this_thread::get_id() << ":" << _actuallyProcessed << std::endl;
    }

private:
    int32_t _actuallyProcessed{ 0 };
};

class Producer
{
public:
    Producer(std::int32_t ringBufferSize, std::int32_t workerCount)
    {
        m_ringBufferSize = ringBufferSize;
        m_workerCount = workerCount;
        // 创建调度器
        m_ptrTaskScheduler = std::make_shared< Disruptor::ThreadPerTaskScheduler >();
        // 创建Disruptor
        m_ptrDisruptor = std::make_shared< Disruptor::disruptor<Event> >([]() { return Event(); }
        , m_ringBufferSize, m_ptrTaskScheduler);
        // 创建事件处理器
        for (size_t i = 0; i < m_workerCount; i++)
        {
            m_workers.push_back(std::make_shared< Worker >());
        }
        // 绑定
        //m_ptrDisruptor->handleEventsWithWorkerPool(m_workers);
        m_ptrDisruptor->handleEventsWithWorkerPool(m_workers)->then(std::make_shared< Worker2 >())->then(std::make_shared< Worker1 >());
        //m_ptrDisruptor->handleEventsWith(std::make_shared< Worker1 >());
        //m_ptrDisruptor->handleEventsWith(std::make_shared< Worker2 >());
    }

    ~Producer()
    {
        stop();
    }

    void start()
    {
        m_ptrTaskScheduler->start();
        m_ptrDisruptor->start();
    }

    void push(const Event &event)
    {
        auto ringBuffer = m_ptrDisruptor->ringBuffer();
        auto nextSequence = ringBuffer->next();
        (*ringBuffer)[nextSequence] = event;
        ringBuffer->publish(nextSequence);
    }

protected:
    void stop()
    {
        m_ptrDisruptor->shutdown();
        m_ptrTaskScheduler->stop();
    }

private:
    std::shared_ptr<Disruptor::ThreadPerTaskScheduler> m_ptrTaskScheduler;
    std::shared_ptr<Disruptor::disruptor<Event> > m_ptrDisruptor;
    std::vector<std::shared_ptr<Disruptor::IWorkHandler<Event> > > m_workers;
    std::int32_t m_ringBufferSize{ 1024 };
    std::int32_t m_workerCount{ 1 };
    std::int32_t m_schedulerCount{ 1 };
};


int main(int argc, char* argv[])
{
    timespec t1,t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);

    if(argc < 4)
        return 0;

    Producer producer(atoi(argv[1]), atoi(argv[2]));
    producer.start();

    for (size_t i = 0; i < atoi(argv[3]); i++)
    {
    	auto e = Event();
		e.id = i;
        producer.push(e);
    }
    clock_gettime(CLOCK_MONOTONIC, &t2);

    std::cout << "event num:" << argv[3] << std::endl;
    std::cout << "worker num:" << argv[2] << std::endl;
    std::cout << "buffer size:" << argv[1] << std::endl;
    std::cout << "time:" << 
    (t2.tv_nsec - t1.tv_nsec) / 1000000 + (t2.tv_sec - t1.tv_sec) * 1000
    << std::endl;
    sleep(10);
    return 0;
}
