#pragma once

#include <stdint.h>

typedef long long int64;
typedef unsigned long long uint64;
typedef unsigned int size32_t;

template<typename T, int tid>
struct TGenericID
{
	T id;

	TGenericID():id(0)
	{
	}

	TGenericID(const T& i):id(i)
	{
	}

	TGenericID& operator = (const T& val)
	{
		id = val;
		return *this;
	}

	bool operator < (const TGenericID& b)const
	{
		return id < b.id;
	}

	bool operator > (const TGenericID& b)const
	{
		return id > b.id;
	}

	TGenericID operator ++()
	{
		++id;
		return *this;
	}

	bool operator != (const TGenericID& val)const
	{
		return id!=val.id;
	}
	bool operator == (const TGenericID& val)const
	{
		return id==val.id;
	}
	bool operator <= (const TGenericID& val)const
	{
		return id<=val.id;
	}
	bool operator >= (const TGenericID& val)const
	{
		return id>=val.id;
	}
};

typedef TGenericID<int64, 2> TPersistID;

