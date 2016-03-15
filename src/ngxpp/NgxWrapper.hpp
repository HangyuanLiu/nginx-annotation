#ifndef _NGX_WRAPPER_HPP
#define _NGX_WRAPPER_HPP

//#include <boost/type_traits.hpp>
#include <type_traits>
#include "Nginx.hpp"

template<typename T>
class NgxWrapper
{
public:
	typedef typename std::remove_pointer<T>::type wrapped_type;
	//typedef typename boost::remove_pointer<T>::type wrapped_type;
	typedef wrapped_type* pointer_type;
	typedef wrapped_type& reference_type;
private:
	pointer_type m_ptr = nullptr;

protected:
	NgxWrapper(pointer_type p):m_ptr(p){}	//参数是指针类型
	NgxWrapper(reference_type x):m_ptr(&x){}	//参数是引用类型
	~NgxWrapper() = default;	//析构函数不做任何事

public:
	pointer_type get() const
	{
		return m_ptr;
	}
	operator bool () const
	{
		return get();
	}
	operator pointer_type () const
	{
		return get();
	}
	pointer_type operator->() const
	{
		return get();
	}
};

#endif //_NGX_WRAPPER_HPP
