//
// Created by ltc on 2021/3/7.
//

#ifndef MYPTHREAD_THREAD_H
#define MYPTHREAD_THREAD_H

#include <pthread.h>
#include <unordered_map>

class Thread {
public:
    Thread();
    void run(void* handle(void*), void* arg);
    bool join();
    bool detach();
    bool cancel();
    pthread_t getID() const;
    bool getState() const;
    void* getResult() const;
	~Thread();
    Thread(const Thread&) = delete;
    Thread(Thread&&) = delete;
    Thread& operator=(const Thread&) = delete;
    Thread& operator=(Thread&&) = delete;
private:
	struct Cache {
		Cache* next;
	};
private:
	friend class ObjPool;
	void* allocate(size_t size);
	bool deallocate(void* ptr, size_t size);
	void* allocateBuffer(size_t size);
	bool deallocateBuffer(void* ptr);
private:
	int num;
	static std::unordered_map<pthread_t, Thread*> ThreadMap;
	std::unordered_map<size_t, Cache*> cache;
	std::unordered_map<size_t, int> cacheNum;
	std::unordered_map<size_t, Cache*> buffCache;
    bool isRunning;
    pthread_t threadID;
    void* result;
};


#endif //MYPTHREAD_THREAD_H
