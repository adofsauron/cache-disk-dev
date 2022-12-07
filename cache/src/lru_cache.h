/**
@file lru_cache.h
@brief LRU缓存
*/

#pragma once

#include <list>
#include <map>
#include <utility>
#include <algorithm>    
#include <functional>   
#include <iostream>
#include <sstream>

#ifdef _HASH_MAP_
#include <unordered_map>
#endif

#include "assert.h"


struct LRU_RT_Info
{
	int limit			= 0;
	int clean_size		= 0;

	int cache			= 0;
	int hit				= 0;
	int miss			= 0;
	int expire			= 0;
	size_t cell			= 0;

	void Dump()
	{
		std::cout << "lru info : "
			<< ", limit = " << limit
			<< ", clean_size = " <<clean_size
			<< ", cache = " << cache
			<< ", hit = " << hit
			<< ", miss = " << miss
			<< ", expire = " << expire
			<< ", cell = " << cell
		<< std::endl;
	}
};

template<typename key_type, typename value_type>
class CLRUCache
{
private:

	struct LRUData : public value_type
	{ 
		bool _del = false;
	};

private:

	// 数据序列
	using ValueContainer = std::list< std::pair<key_type, LRUData> >;
	using ValueContainer_iterator = typename ValueContainer::iterator;
	using ConstValueContainerIter = typename ValueContainer::const_iterator;

	// 数据索引

	template <typename I_K, typename I_V>
	using IndexType =
#ifdef _HASH_MAP_
		typename std::unordered_map <I_K, I_V>;
#else
		typename std::map<I_K, I_V>;
#endif

	using IndexContainer = IndexType<key_type, ValueContainer_iterator>;
	using IndexContainer_iterator = typename IndexContainer::iterator;
    
public:

	/**
	@brief 构造函数
	@param limit 缓存容量
	@param clean_size 到达容量后,一次清理的个数
	@param expire_time 过期时间,超过该时间视为过期,访问时删除. -1: 永不过期
	@return 
	*/
	CLRUCache(int limit = 10000, int clean_size = 100, time_t expire_time = -1)
		: m_limit ( limit )
		, m_clean_size ( clean_size )
	{

		assert( m_limit > 0 );
		assert( m_clean_size > 0 );

		assert( m_limit >= clean_size );

#ifdef _HASH_MAP_
		m_indexs.reserve(m_limit);
#endif
	}
	
	/**
	@brief 拿第一个元素
	@praram removed 用于保存被删除的值
	@return
	*/
	value_type* Front();

	/**
	@brief 添加缓存项到序列首部项
	@praram removed 用于保存被删除的值
	@return 
	*/
	void Add( key_type key, const value_type& value, value_type* removed = nullptr);
	
	/**
	@brief 批量添加缓存
	@praram
	@return
	*/
	void AddVec(const std::vector<std::pair<key_type, value_type>>& kv_vec);

	/**
	@brief 删除一个缓存项
	@return 
	*/
	void Remove(key_type key) { Remove(key, nullptr); };
	
	/**
	@brief 删除一个缓存项
	@param valueptr 保存被删除的值
	@return 
	*/
	void Remove(key_type key, value_type* valueptr);
	
	/**
	@brief 指定位置删除一个缓存项
	@param pos 删除缓存的位置
	@param valueptr 保存被删除的值
	@return
	*/
	bool RemovePosValue(ValueContainer_iterator& pos, value_type* valueptr = nullptr );

	bool RemovePosIndex(IndexContainer_iterator& pos, value_type* valueptr = nullptr);

	/**
	@brief 从序列首部删除一个缓存项
	@param valueptr 保存被删除的值
	@return 
	*/
	bool RemoveFirst(value_type* valueptr = nullptr) { return RemovePosValue(m_values.begin(), valueptr); };

	/**
	@brief 从序列尾部删除一个缓存项
	@param valueptr 保存被删除的值
	@return
	*/
	bool RemoveLast( value_type* valueptr = nullptr ) { return RemovePosValue(--m_values.end(), valueptr); };
	
	/**
	@brief 清空缓存，重置缓存统计数据
	@return 
	*/
	void Clear();

	/**
	@brief 清理空间,先按LRU清理,再清理过期
	@return
	*/
	void CleanCapacity();

	/**
	@brief 按LRU规则清理空间
	@return
	*/
	void CleanByLRU();

	/**
	@brief 清理应被删除的元素,包含标记删除和过期元素
	@return
	*/
	void CleanNeedDel();
	
	/**
	@brief 查找一个缓存项，如果存在，该缓存项会被放到序列首部
	@return 
	*/
	value_type* Find( key_type key );
	
	/**
	@brief 查找一个缓存项
	@return 
	*/
	value_type* Peek( key_type key );
	
	/**
	@brief 查看缓存项数量
	@return 
	*/
	int Size() const { return m_cache_count; }
	
	/**
	@brief 查看命中次数
	@return 
	*/
	int Hit() const { return m_hit_count; }
	
	/**
	@brief 查看未命中次数
	@return 
	*/
	int Miss() const { return m_miss_count; }

	/**
	@brief 查看失效次数
	@return
	*/
	int ExpireCount() const { return m_expire_count; }
	
	/**
	@brief 查看缓存容量
	@return 
	*/
    int GetLimit() const{ return m_limit; }

	/**
	@brief 查看清理容量的数量设定
	@return
	*/
	int GetCleanSize() const { return m_clean_size; }

	/**
	@brief 设置缓存容量, 只允许增大
	@return
	*/
	void SetLimit(int limit) { m_limit = (limit <= m_limit) ? m_limit : limit; };
	
	/**
	@brief 遍历函数模板
	@return 
	*/
    template <typename FunctorT>
    void ForEach(FunctorT _Func);

    template <typename FunctorT>
    void ForEach(FunctorT _Func) const;
	
	/**
	@brief 输出缓存项内容到标准输出，要求key和value支持ostream <<运算
	@return 
	*/
	void Dump() const;

    //从中随机出count个元素到container中，不够则有多少给多少
    typedef std::map<key_type, value_type>  ElemMap;
    void Rand(ElemMap &container, int count) const;

	/**
	@brief 获取缓存索引
	@return 
	*/
	const IndexContainer& GetIndex() { return m_indexs; };

	/**
	@brief 获取运行时数据
	@return
	*/
	void GetRTInfo(LRU_RT_Info& lru_rt_info);

	/**
	@brief 标记删除,真正删除时机在清理容量时
	@return
	*/
	void MarkDelete(key_type key);

private:
	
	/**
	@brief Forward
	@return
	*/
	bool Forward(ConstValueContainerIter& itr, int step) const
	{
		while((step > 0) && (itr != m_values.end()))
		{
			++itr;
			--step;
		}

		return itr != m_values.end();
	}

private: // 拷贝构造函数和赋值构造函数不允许
	CLRUCache(const CLRUCache&) = delete;
	CLRUCache& operator = (const CLRUCache&) = delete;

private:

	ValueContainer m_values;	///< 缓存序列，保存了缓存项的值
	IndexContainer m_indexs;	///< 缓存索引，保存了缓存序列的迭代器

	int m_limit = 0;
	int m_clean_size = 0;
	int m_cache_count = 0;
	int m_hit_count = 0;
	int m_miss_count = 0;
	int m_expire_count = 0;

	static constexpr size_t m_cell = sizeof(LRUData);
};

template<typename key_type, typename value_type>
void CLRUCache<key_type, value_type>::Dump() const
{	
	std::stringstream ss;

	ss << "limit = " << m_limit << ", clean_size = " 
		<< m_clean_size;
	ss << ", cache_count = " << m_cache_count 
		<< ", hit_count = " << m_hit_count << ", miss_count = " << m_miss_count;
	ss << ", expire_count = " << m_expire_count;

	std::cout << ss.str() << std::endl;

	// 每一条数据写一次日志,避免内存占用过大
	for (const auto& kv : m_values)
	{
		// ss.str("");
		// ss << kv.first << "\t: " << kv.second;
		// std::cout << ss.str() << std::endl;
	}
}

template<typename key_type, typename value_type>
bool CLRUCache<key_type, value_type>::RemovePosValue(ValueContainer_iterator& pos, value_type* valueptr)
{
	if (pos == m_values.end())
	{
		return false;
	}

	if (valueptr)
	{
		*valueptr = pos->second;
	}

	m_indexs.erase(pos->first);
	m_values.erase(pos);

	--m_cache_count;

	return true;
}

template<typename key_type, typename value_type>
bool CLRUCache<key_type, value_type>::RemovePosIndex(IndexContainer_iterator& pos, value_type* valueptr)
{
	if (pos == m_indexs.end())
	{
		return false;
	}

	ValueContainer_iterator v_pos = pos->second;

	if (valueptr)
	{
		*valueptr = v_pos->second;
	}

	m_indexs.erase(pos);
	m_values.erase(v_pos);

	--m_cache_count;

	return true;
}

template<typename key_type, typename value_type>
void CLRUCache<key_type, value_type>::Clear()
{
	m_indexs.clear();
	m_values.clear();

	m_cache_count = 0;
	m_hit_count = 0;
	m_miss_count = 0;
	m_expire_count = 0;
}

template<typename key_type, typename value_type>
void CLRUCache<key_type, value_type>::CleanCapacity()
{
	CleanNeedDel();
	CleanByLRU();
}

template<typename key_type, typename value_type>
void CLRUCache<key_type, value_type>::CleanByLRU()
{
	if (m_cache_count < m_clean_size)
	{
		return;
	}

	int num = m_clean_size;
	while (num--)
	{
		RemoveLast();
	}
}

template<typename key_type, typename value_type>
value_type* CLRUCache<key_type, value_type>::Find( key_type key )
{
	IndexContainer_iterator itr = m_indexs.find( key );

	if (itr == m_indexs.end())
	{
		++m_miss_count;
		return nullptr;
	}

	m_values.splice(m_values.begin(), m_values, itr->second);

	LRUData& data = itr->second->second;
	value_type* result = (value_type*)(&data);
	return result;
}

template<typename key_type, typename value_type>
value_type* CLRUCache<key_type, value_type>::Peek( key_type key )
{
	IndexContainer_iterator itr = m_indexs.find( key );

	if (itr == m_indexs.end())
	{
		return nullptr;
	}

	LRUData& data = itr->second->second;
	value_type* result = (value_type*)(&data);

	return result;
}

template<typename key_type, typename value_type>
void CLRUCache<key_type, value_type>::Add( key_type key, const value_type& value, value_type* removed )
{
	IndexContainer_iterator itr = m_indexs.find( key );

	if ( itr == m_indexs.end() )
	{
		if ( m_cache_count >= m_limit ) // 需要返回则删末尾并返回, 否则按配置清理空间
		{
			removed ? (void) RemoveLast(removed) : CleanCapacity();
		}

		LRUData data;
		data._del = false;

		value_type* data_base = &data;
		*data_base = value;

		m_values.emplace_front( std::make_pair( key, data ) );
		m_indexs.emplace(key, m_values.begin());

		++m_cache_count;
	}
	else
	{
		LRUData& data = itr->second->second;

		value_type* data_base = &data;
		*data_base = value;

		data._del = false;
	}
}

template <typename key_type, typename value_type> template <typename FunctorT>
void CLRUCache<key_type, value_type>::ForEach(FunctorT _Func)
{
	std::for_each(m_values.begin(), m_values.end(), _Func);
}

template <typename key_type, typename value_type> template <typename FunctorT>
void CLRUCache<key_type, value_type>::ForEach(FunctorT _Func) const
{
	std::for_each(m_values.begin(), m_values.end(), _Func);
}

template<typename key_type, typename value_type>
void CLRUCache<key_type, value_type>::Remove(key_type key, value_type* valueptr)
{ 
	IndexContainer_iterator iter = m_indexs.find(key);
	if (iter == m_indexs.end())
	{
		return;
	}

	RemovePosIndex(iter, valueptr); 
};

template<typename key_type, typename value_type>
void CLRUCache<key_type, value_type>::GetRTInfo(LRU_RT_Info& lru_rt_info)
{
	lru_rt_info.limit			= m_limit;
	lru_rt_info.clean_size		= m_clean_size;

	lru_rt_info.cache			= m_cache_count;
	lru_rt_info.hit				= m_hit_count;
	lru_rt_info.miss			= m_miss_count;
	lru_rt_info.expire			= m_expire_count;
	lru_rt_info.cell			= m_cell;
}

template<typename key_type, typename value_type>
value_type* CLRUCache<key_type, value_type>::Front()
{
	if (m_cache_count <= 0)
	{
		return nullptr;
	}

	std::pair<key_type, LRUData>& value = m_values.front();

	LRUData& data = value.second;
	return (value_type*)(&data);
}

template<typename key_type, typename value_type>
void CLRUCache<key_type, value_type>::AddVec(const std::vector<std::pair<key_type, value_type>>& kv_vec)
{
	if (kv_vec.empty())
	{
		return;
	}

	for_each(kv_vec.begin(), kv_vec.end(), [this](const std::pair<key_type, value_type>& kv) {
		this->Add(kv.first, kv.second);
	});
}

template<typename key_type, typename value_type>
void CLRUCache<key_type, value_type>::MarkDelete(key_type key)
{
	IndexContainer_iterator itr = m_indexs.find(key);
	if (itr == m_indexs.end())
	{
		return;
	}

	itr->second->second._del = true;
}

template<typename key_type, typename value_type>
void CLRUCache<key_type, value_type>::CleanNeedDel()
{
	std::vector<IndexContainer_iterator> del_elements;

	for (IndexContainer_iterator itr = m_indexs.begin(); itr != m_indexs.end(); ++itr)
	{
		if (itr->second->second._del) // 如果标记删除,则无论过期都应删除
		{
			del_elements.emplace_back(itr);
			continue;
		}
	}

	if (del_elements.empty())
	{
		return;
	}

	for (auto& itr : del_elements)
	{
		RemovePosIndex(itr);
	}
}