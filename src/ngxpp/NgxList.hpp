// Copyright (c) 2015
// Author: Chrono Law
#ifndef _NGX_LIST_HPP
#define _NGX_LIST_HPP

#include <boost/iterator/iterator_facade.hpp>
#include <boost/core/explicit_operator_bool.hpp>

#include "NgxPool.hpp"


template<typename T>
class NgxListIterator
{
public:
	NgxListIterator(ngx_list_t* l):
		m_part(&l->part),
		m_data(static_cast<T*>(m_part->elts)){}
	NgxListIterator() = default;
	~NgxListIterator() = default;
	NgxListIterator(NgxListIterator const&) = default;
	NgxListIterator& operator=(const NgxListIterator&) = default;
    bool operator!() const
    {
        return !m_part || !m_data || !m_part->nelts;
    }
    T& operator++() {
    	++m_count;
    	if(m_count >= m_part->nelts)
    	{
    		m_count = 0;
    		m_part = m_part->next;
    		m_data = m_part?
    				static_cast<T*>(m_part->elts):nullptr;
    	}
    	return this->m_data[m_count];
    }
    T& operator* (){
    	NgxException::require(m_data);
    	return this->m_data[m_count];
    }
    bool operator==(NgxListIterator it) const
	{
        return m_part == it.m_part &&
               m_data == it.m_data &&
               m_count == it.m_count;
	}
private:
    ngx_list_part_t* m_part = nullptr;
    T* m_data = nullptr;
    ngx_uint_t m_count = 0;
};

template<typename T>
class NgxList final : public NgxWrapper<ngx_list_t>
{
public:
    typedef NgxWrapper<ngx_list_t> super_type;
    typedef NgxList this_type;

    typedef T value_type;
public:
    NgxList(const NgxPool& p, ngx_uint_t n = 10):
        super_type(p.list<T>(n))
    {}

    NgxList(ngx_list_t* l):super_type(l)
    {}

    NgxList(ngx_list_t& l):super_type(&l)
    {}

    ~NgxList() = default;
public:
    T& prepare() const
    {
        auto tmp = ngx_list_push(get());

        NgxException::require(tmp);

        assert(tmp);
        return *reinterpret_cast<T*>(tmp);
    }

    void push(const T& x) const
    {
        prepare() = x;
    }
public:
    bool empty() const
    {
    	return !get()->part.nelts;
        //return !get()->part->nelts;
    }
public:
    void merge(const this_type& l) const
    {
        auto f = [this](const value_type& v)
        {
            prepare() = v;
        };

        l.visit(f);
    }
public:
    typedef NgxListIterator<T> iterator;
    typedef const iterator const_iterator;

    iterator begin() const
    {
        return iterator(get());
    }

    iterator end() const
    {
        return iterator();
    }
public:
    template<typename V>
    void visit(V v) const
    {
        auto iter = begin();
        for(; iter; ++iter)
        {
            v(*iter);
        }
    }
public:
    template<typename Predicate>
    iterator find(Predicate p) const
    {
    	auto iter = begin();
    	for(auto iter = begin();iter;++iter)
    	{
    		if(p(*iter)) return iter;
    	}
    	return end();
    }
};

#endif  //_NGX_LIST_HPP
