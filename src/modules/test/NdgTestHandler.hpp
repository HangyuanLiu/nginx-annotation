#ifndef	_NDG_TEST_HANDLER_HPP
#define	_NDG_TEST_HANDLER_HPP

#include <iostream>
#include "NdgTestConf.hpp"

class NdgTestHandler final
{
public:
	typedef NdgTestHandler this_type;	//自身类型定义
public:
	static ngx_int_t init(ngx_conf_t* cf)
	{
		auto& cmcf = NgxHttpCoreModule::instance().conf().main(cf);
		NgxArray<ngx_http_handler_pt> arr
			(cmcf.phases[NGX_HTTP_REWRITE_PHASE].handlers);
		arr.push(this_type::handler);
		return NGX_OK;
	}
public:
	static ngx_int_t handler(ngx_http_request_t	*r)
	try{
		NgxClock clock;
		auto& cf = NdgTestModule::instance().conf().loc(r);
		NgxLogError(r).print("hello nginx");

		if(!cf.enabled)
		{
			std::cout<<"hello disabled"<<std::endl;
			return NGX_DECLINED;
		}
		std::cout<<"hello nginx"<<std::endl;

		std::cout<<clock.elapsed()<<"s"<<std::endl;
		return NGX_DECLINED;
	}
	catch(const NgxException& e)
	{
		return e.code();	//返回错误码
	}
};
#endif
