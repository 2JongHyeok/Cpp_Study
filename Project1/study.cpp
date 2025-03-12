#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <concurrent_queue.h>
#include <chrono>

using namespace std;

constexpr int MAX_THREADS = 16;

class Lock_Queue {
	queue<int> q;
	mutex ql;
public:
	void Enqueue(int x) {
		ql.lock();
		q.push(x);
		ql.unlock();
	}
	int Dequeue() {
		ql.lock();
		if (q.empty()) {
			ql.unlock();
			return -1;
		}
		int val = q.front();
		q.pop();
		ql.unlock();
		return val;
	}

	void Clear() {
		while (!q.empty()) q.pop();
	}

	void Print20() {
		for (int i = 0; i < 20; ++i) {
			int v = Dequeue();
			if (-1 == v) break;
			std::cout << v << ", ";
		}
		std::cout << std::endl;
	}
};

class Concurrent_Queue {
	concurrency::concurrent_queue<int> q;
public:
	void Enqueue(int x) {
		q.push(x);
	}
	int Dequeue() {
		int val;
		bool succ;
		succ = q.try_pop(val);
		if (succ)
			return val;
		else
			return -1;
	}

	void Clear() {
		q.clear();
	}

	void Print20() {
		for (int i = 0; i < 20; ++i) {
			int v = Dequeue();
			if (-1 == v) break;
			std::cout << v << ", ";
		}
		std::cout << std::endl;
	}
};

class NODE {
public:
	int	key;
	NODE* volatile next;
	NODE(int x) : key(x), next(nullptr) {}
};

class LF_QUEUE {
	NODE* volatile head;
	NODE* volatile tail;
public:
	LF_QUEUE()
	{
		head = tail = new NODE{ -1 };
	}
	void Clear()
	{
		while (-1 != Dequeue());
	}
	bool CAS(NODE* volatile* ptr, NODE* old_ptr, NODE* new_ptr)
	{
		return std::atomic_compare_exchange_strong(
			reinterpret_cast<volatile std::atomic_llong*>(ptr),
			reinterpret_cast<long long*>(&old_ptr),
			reinterpret_cast<long long>(new_ptr)
		);
	}
	void Enqueue(int x)
	{
		NODE* n = new NODE(x);
		while (true) {
			NODE* last = tail;
			NODE* next = last->next;
			if (last != tail) continue;
			if (next != nullptr) {
				CAS(&tail, last, next);
				continue;
			}
			if (true == CAS(&last->next, nullptr, n)) {
				CAS(&tail, last, n);
				return;
			}
		}

	}
	int Dequeue()
	{
		while (true) {
			NODE* first = head;
			NODE* last = tail;
			NODE* next = first->next;
			if (first != head) continue;
			if (nullptr == next) return -1;
			if (first == last) {
				CAS(&tail, last, next);
				continue;
			}
			int value = next->key;
			if (false == CAS(&head, first, next)) continue;
			return value;
		}
	}
	void Print20()
	{
		for (int i = 0; i < 20; ++i) {
			int v = Dequeue();
			if (-1 == v) break;
			std::cout << v << ", ";
		}
		std::cout << std::endl;
	}
};

LF_QUEUE my_queue;

thread_local int thread_id;

const int NUM_TEST = 4'000'000;

std::atomic_int loop_count = NUM_TEST;

void benchmark(const int th_id, const int num_thread)
{
	int key = 0;

	thread_id = th_id;
	while (loop_count-- > 0) {
		if (rand() % 2 == 0)
			my_queue.Enqueue(key++);
		else
			my_queue.Dequeue();
	}
}

int main()
{
	using namespace std::chrono;

	for (int n = 1; n <= MAX_THREADS; n = n * 2) {
		loop_count = NUM_TEST;
		my_queue.Clear();
		std::vector<std::thread> tv;
		auto start_t = high_resolution_clock::now();
		for (int i = 0; i < n; ++i) {
			tv.emplace_back(benchmark, i, n);
		}
		for (auto& th : tv)
			th.join();
		auto end_t = high_resolution_clock::now();
		auto exec_t = end_t - start_t;
		size_t ms = duration_cast<milliseconds>(exec_t).count();
		std::cout << n << " Threads,  " << ms << "ms.";
		my_queue.Print20();
	}
}