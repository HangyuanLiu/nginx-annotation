#ifndef _NDG_TEST_CONF_HPP
#define _NDG_TEST_CONF_HPP

#include "NgxAll.hpp"

class NdgTestConf final	//禁止被继承
{
public:
	typedef NdgTestConf this_type;
public:
	NdgTestConf() = default;
	~NdgTestConf() = default;
public:
	ngx_flag_t enabled = NgxUnsetValue::get();
public:
	static void* create(ngx_conf_t* cf)
	{
		return NgxPool(cf).alloc<this_type>();
	}
	static char* merge(ngx_conf_t *cf,void *parent,void *child)
	{
		auto prev = reinterpret_cast<this_type*>(parent);
		auto conf = reinterpret_cast<this_type*>(child);

		NgxValue::merge(conf->enabled,prev->enabled,0);		//合并变量,默认值为0

		return NGX_CONF_OK;
	}
};

struct NdgTestModule{

	static NgxModule<NdgTestConf>& instance(){
		extern ngx_module_t ndg_test_module;
		static NgxModule<NdgTestConf> m(ndg_test_module);
		return m;
	}
	NgxModule<NdgTestConf>& operator()() const
	{
		return instance();
	}
};
#endif
