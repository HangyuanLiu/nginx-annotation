
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HTTP_CONFIG_H_INCLUDED_
#define _NGX_HTTP_CONFIG_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

/*
 * 当Nginx检测到http{}这个关键配置项时，HTTP配置模型就启动了，
 * 这时会首先建立1个ngx_http_conf_ctx_t结构
 * 这个结构体不仅http配置项使用，srv和loc都会建立各自的该型结构体。详见《深入理解》中的配置内存分配图
 * */
typedef struct {
	/* 指针数组，数组中的每个元素指向所有HTTP模块create_main_conf方法产生的结构体
	 * 它们存放着解析直属http{}块内的main级别的配置项参数
	 * */
    void        **main_conf;
    /*指针数组，数组中的每个元素指向所有HTTP模块create_srv_conf方法产生的结构体*/
    void        **srv_conf;
    /*指针数组，数组中的每个元素指向所有HTTP模块create_loc_conf方法产生的结构体*/
    void        **loc_conf;
} ngx_http_conf_ctx_t;

/*
 * HTTP框架在读取,重载配置文件时定义了由ngx_http_module_t接口描述的8个阶段
 * 框架在启动过程中会在每个阶段中调用ngx_http_module_t中相应的方法
 * http框架会循环调用每个模块中的回调函数，
 * */
typedef struct {
	//解析http{}配置项前调用
    ngx_int_t   (*preconfiguration)(ngx_conf_t *cf);
    //解析http{}内所以配置项后回调
    ngx_int_t   (*postconfiguration)(ngx_conf_t *cf);

    /* 回调方法，负责把我们分配的用于保存配置项的结构体传递给HTTP框架
     * 根据index序号，存储在配置项里,它会在解析main配置项前调用
     * 创建用于存储http全局配置项的结构体,该结构体中的成员将保存直属于http{}块的配置项参数
     */
    void       *(*create_main_conf)(ngx_conf_t *cf);
    //解析完main配置项后回调,常用于初始化main级别配置项
    char       *(*init_main_conf)(ngx_conf_t *cf, void *conf);
    /* 创建用于存储可同时出现在main,srv级别配置项的结构体，该结构体中的成员与server配置是
     * 相关联的
     * */
    void       *(*create_srv_conf)(ngx_conf_t *cf);
    //merge_srv_conf主要用于合并main级别和srv级别下的同名配置项
    char       *(*merge_srv_conf)(ngx_conf_t *cf, void *prev, void *conf);
    /* 创建用于存储可同时出现在main,srv,loc级别配置项的结构体,该结构体
     * 中的成员与location配置项是相关联的
     * */
    void       *(*create_loc_conf)(ngx_conf_t *cf);
    /* create_loc_conf产生的结构体所要解析的配置项,可能同时出现在main,srv,loc级别中
     * merge_loc_conf方法可以分别把出现在main,srv级别的配置项值合并到loc级别的配置项中
     * */
    char       *(*merge_loc_conf)(ngx_conf_t *cf, void *prev, void *conf);
} ngx_http_module_t;


#define NGX_HTTP_MODULE           0x50545448   /* "HTTP" */

#define NGX_HTTP_MAIN_CONF        0x02000000
#define NGX_HTTP_SRV_CONF         0x04000000
#define NGX_HTTP_LOC_CONF         0x08000000
#define NGX_HTTP_UPS_CONF         0x10000000
#define NGX_HTTP_SIF_CONF         0x20000000
#define NGX_HTTP_LIF_CONF         0x40000000
#define NGX_HTTP_LMT_CONF         0x80000000

//使用create_main_conf方法产生的结构体来存储解析出的配置项参数
#define NGX_HTTP_MAIN_CONF_OFFSET  offsetof(ngx_http_conf_ctx_t, main_conf)
#define NGX_HTTP_SRV_CONF_OFFSET   offsetof(ngx_http_conf_ctx_t, srv_conf)
#define NGX_HTTP_LOC_CONF_OFFSET   offsetof(ngx_http_conf_ctx_t, loc_conf)


#define ngx_http_get_module_main_conf(r, module)                             \
    (r)->main_conf[module.ctx_index]
#define ngx_http_get_module_srv_conf(r, module)  (r)->srv_conf[module.ctx_index]
#define ngx_http_get_module_loc_conf(r, module)  (r)->loc_conf[module.ctx_index]


#define ngx_http_conf_get_module_main_conf(cf, module)                        \
    ((ngx_http_conf_ctx_t *) cf->ctx)->main_conf[module.ctx_index]
#define ngx_http_conf_get_module_srv_conf(cf, module)                         \
    ((ngx_http_conf_ctx_t *) cf->ctx)->srv_conf[module.ctx_index]
#define ngx_http_conf_get_module_loc_conf(cf, module)                         \
    ((ngx_http_conf_ctx_t *) cf->ctx)->loc_conf[module.ctx_index]

/* cycle是ngx_cycle_t核心结构体指针，而module则是所要操作的http模块
 * */
#define ngx_http_cycle_get_module_main_conf(cycle, module)                    \
    (cycle->conf_ctx[ngx_http_module.index] ?                                 \
        ((ngx_http_conf_ctx_t *) cycle->conf_ctx[ngx_http_module.index])      \
            ->main_conf[module.ctx_index]:                                    \
        NULL)


#endif /* _NGX_HTTP_CONFIG_H_INCLUDED_ */
