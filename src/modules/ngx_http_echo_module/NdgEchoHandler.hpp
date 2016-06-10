// Copyright (c) 2015
// Author: Chrono Law
#ifndef _NDG_ECHO_HANDLER_HPP
#define _NDG_ECHO_HANDLER_HPP

#include <iostream>

#include "NdgEchoConf.hpp"

class NdgEchoHandler final{
public:
	static ngx_int_t handler(ngx_http_request_t *r)
	{
		return process(r);
	}
private:
	static ngx_int_t process(ngx_http_request_t *r)
	{
		try{
			NgxRequest req(r);	//请求对象
			if(!req.method(NGX_HTTP_GET))
			{
				return NGX_HTTP_NOT_ALLOWED;
			}
			req.body().discard();

			auto& cf = NdgEchoModule::instance().conf().loc(r);
			NgxString msg = cf.msg;
			NgxString args = req->args;
			auto len = msg.size();
			if(!args.empty())
			{
				len += args.size() + 1;
			}

			NgxResponse resp(r);
			resp.length(len);
			resp.status(NGX_HTTP_OK);

			if(!args.empty()){
				resp.send(args);
				resp.send(",");
			}
			resp.send(msg);
			return resp.eof();
		}
		catch(const NgxException& e)
		{
			return e.code();
		}
	}
};

#endif  //_NDG_ECHO_HANDLER_HPP
