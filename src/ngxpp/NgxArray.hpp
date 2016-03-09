#ifndef _NGX_ARRAY_HPP
#define _NGX_ARRAY_HPP

#include "NgxPool.hpp"
template<typename T>
class NgxArray final:public NgxWrapper<ngx_array_t>
{
public:
	typedef NgxWrapper<ngx_array_t> super_type;
	typedef NgxArray this_type;

public:
	NgxArray(const NgxPool &p,ngx_uint_t n = 10):
		super_type(p.array<T>(n))		//调用内存池创建数组
	{}
	NgxArray(ngx_array_t* arr):super_type(arr){}	//指针参数
	NgxArray(ngx_array_t& arr):super_type(arr){}	//引用参数
	~NgxArray() = default;	//默认析构函数
public:
	ngx_uint_t size() const		//获得数组大小
	{
		return get()?get()->nelts:0;	//防止空指针
	}
	T& operator[](ngx_uint_t i) const	//重载操作符[]
	{
		NgxException::require(i<size()&&get());
		return elts()[i];
	}
public:
	template<typename V>
	void visit(V v) const
	{
		auto p = elts();
		for(ngx_uint_t i = 0 ;i < size();++i)
		{
			v(p[i]);
		}
	}

public:
	T& prepare() const
	{
		auto tmp = ngx_array_push(get());
		NgxException::require(tmp);
		return *reinterpret_cast<T*>(tmp);
	}
	void push(const T& x) const
	{
		prepare() = x;
	}
private:
	T* elts() const
	{
		return reinterpret_cast<T*>(get()->elts);
	}
};

typedef NgxArray<ngx_str_t> NgxStrArray;
#endif
