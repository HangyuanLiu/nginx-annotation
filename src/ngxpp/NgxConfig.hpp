#ifndef _NGX_CONFIG_HPP
#define _NGX_CONFIG_HPP

#include "NgxArray.hpp"
class NgxCommand final
{
private:
	ngx_command_t m_cmd = ngx_null_command;
public:
	template<typename T>
	NgxCommand(const ngx_str_t& n,
				ngx_uint_t t,
				T set,
				ngx_uint_t c = NGX_HTTP_LOC_CONF_OFFSET,
				ngx_uint_t off = 0,
				void* p = nullptr):
				m_cmd{n,t,set,c,off,p}
	{}

	NgxCommand() = default;
	~NgxCommand() = default;
public:
	operator const ngx_command_t& () const
	{
		return m_cmd;
	}
};
class NgxTake final
{
public:
	NgxTake(ngx_uint_t conf,int m,int n = -1):
		m_type(conf|take(m,n)){}
	~NgxTake() = default;
public:
	operator ngx_uint_t () const
	{
		return m_type;
	}
private:
	ngx_uint_t m_type = NGX_HTTP_LOC_CONF;
private:
	static ngx_uint_t take(int m,int n)
	{
		static
		ngx_uint_t takes[] = {
				NGX_CONF_NOARGS,
				NGX_CONF_TAKE1,
				NGX_CONF_TAKE2,
				NGX_CONF_TAKE3,
				NGX_CONF_TAKE4,
				NGX_CONF_TAKE5,
				NGX_CONF_TAKE6,
				NGX_CONF_TAKE7
		};
		if(n<0||n>m)	//参数有效性检查
		{
			return takes[m];
		}
		if(n >= NGX_CONF_MAX_ARGS)
		{
			return m == 1?NGX_CONF_1MORE:NGX_CONF_2MORE;
		}

		ngx_uint_t tmp = 0;
		for(int i = m; i<=n;++i)
		{
			tmp |= takes[i];
		}
		return tmp;
	}
};

class NgxConfig final
{
public:
	NgxConfig() = default;
	~NgxConfig() = default;
public:
	static NgxStrArray args(ngx_conf_t* cf)
	{
		return NgxStrArray(cf->args);
	}
};

#include <boost/preprocessor/repetition/enum.hpp>
#define NGX_NULL_HELPER(z,n,t) t
#define NGX_MODULE_NULL(n) BOOST_PP_ENUM(n,NGX_NULL_HELPER,nullptr)

#endif
