#ifndef _NDG_ECHO_CONF_HPP
#define _NDG_ECHO_CONF_HPP

#include "NgxAll.hpp"

class NdgEchoConf final
{
public:
	typedef NdgEchoConf this_type;
	NdgEchoConf() = default;
	~NdgEchoConf() = default;

	ngx_str_t msg;
	static void* create(ngx_conf_t* cf)
	{
		return NgxPool(cf).alloc<this_type>();
	}
};

struct NdgEchoModule{
	static NgxModule<NdgEchoConf>& instance()
	{
		extern ngx_module_t mod;
		static NgxModule<NdgEchoConf> m(mod);
		return m;
	}
	NgxModule<NdgEchoConf> operator()() const
	{
		return instance();
	}
};
#endif  //_NDG_ECHO_CONF_HPP
