#include <iostream>

#include "lru_cache.h"
#include "base.h"


class Data
{
public:
	Data() {}

	Data(TPersistID _pstid, int _age)
		: pstid(_pstid)
		, age(_age)
	{
	}

	TPersistID pstid = 0;
	int age = 0;
};

static void CacheInitData(CLRUCache<TPersistID, Data>& cache, int size)
{
	cache.Clear();
	for (int i = 0; i < size; ++i)
	{
		Data data{ i + 10001, i + 101 };
		cache.Add(data.pstid, data);
	}
}

int main(int argc, const char** argv) {


    CLRUCache<TPersistID, Data> cache{ LCT_START };

	constexpr int size = 5;
	CacheInitData(cache, size);
	cache.Dump();


    LRU_RT_Info info;
    cache.GetRTInfo(info);

    // info.Dump();


    return 0;
}

