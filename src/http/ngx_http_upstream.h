
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HTTP_UPSTREAM_H_INCLUDED_
#define _NGX_HTTP_UPSTREAM_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_event_connect.h>
#include <ngx_event_pipe.h>
#include <ngx_http.h>


#define NGX_HTTP_UPSTREAM_FT_ERROR           0x00000002
#define NGX_HTTP_UPSTREAM_FT_TIMEOUT         0x00000004
#define NGX_HTTP_UPSTREAM_FT_INVALID_HEADER  0x00000008
#define NGX_HTTP_UPSTREAM_FT_HTTP_500        0x00000010
#define NGX_HTTP_UPSTREAM_FT_HTTP_502        0x00000020
#define NGX_HTTP_UPSTREAM_FT_HTTP_503        0x00000040
#define NGX_HTTP_UPSTREAM_FT_HTTP_504        0x00000080
#define NGX_HTTP_UPSTREAM_FT_HTTP_403        0x00000100
#define NGX_HTTP_UPSTREAM_FT_HTTP_404        0x00000200
#define NGX_HTTP_UPSTREAM_FT_UPDATING        0x00000400
#define NGX_HTTP_UPSTREAM_FT_BUSY_LOCK       0x00000800
#define NGX_HTTP_UPSTREAM_FT_MAX_WAITING     0x00001000
#define NGX_HTTP_UPSTREAM_FT_NOLIVE          0x40000000
#define NGX_HTTP_UPSTREAM_FT_OFF             0x80000000

#define NGX_HTTP_UPSTREAM_FT_STATUS          (NGX_HTTP_UPSTREAM_FT_HTTP_500  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_502  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_503  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_504  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_403  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_404)

#define NGX_HTTP_UPSTREAM_INVALID_HEADER     40


#define NGX_HTTP_UPSTREAM_IGN_XA_REDIRECT    0x00000002
#define NGX_HTTP_UPSTREAM_IGN_XA_EXPIRES     0x00000004
#define NGX_HTTP_UPSTREAM_IGN_EXPIRES        0x00000008
#define NGX_HTTP_UPSTREAM_IGN_CACHE_CONTROL  0x00000010
#define NGX_HTTP_UPSTREAM_IGN_SET_COOKIE     0x00000020
#define NGX_HTTP_UPSTREAM_IGN_XA_LIMIT_RATE  0x00000040
#define NGX_HTTP_UPSTREAM_IGN_XA_BUFFERING   0x00000080
#define NGX_HTTP_UPSTREAM_IGN_XA_CHARSET     0x00000100
#define NGX_HTTP_UPSTREAM_IGN_VARY           0x00000200


typedef struct {
    ngx_msec_t                       bl_time;
    ngx_uint_t                       bl_state;

    ngx_uint_t                       status;
    time_t                           response_sec;
    ngx_uint_t                       response_msec;
    time_t                           header_sec;
    ngx_uint_t                       header_msec;
    off_t                            response_length;

    ngx_str_t                       *peer;
} ngx_http_upstream_state_t;


typedef struct {
    ngx_hash_t                       headers_in_hash;
    ngx_array_t                      upstreams;
                                             /* ngx_http_upstream_srv_conf_t */
} ngx_http_upstream_main_conf_t;

typedef struct ngx_http_upstream_srv_conf_s  ngx_http_upstream_srv_conf_t;

typedef ngx_int_t (*ngx_http_upstream_init_pt)(ngx_conf_t *cf,
    ngx_http_upstream_srv_conf_t *us);
typedef ngx_int_t (*ngx_http_upstream_init_peer_pt)(ngx_http_request_t *r,
    ngx_http_upstream_srv_conf_t *us);


typedef struct {
	//配置解析时初始化
    ngx_http_upstream_init_pt        init_upstream;
    //请求时初始化
    ngx_http_upstream_init_peer_pt   init;
    //服务器列表指针
    void                            *data;
} ngx_http_upstream_peer_t;

//存放了upstream配置块里每个服务器的信息,供模块实时参考
typedef struct {
    ngx_str_t                        name;
    ngx_addr_t                      *addrs;	//地址数组
    ngx_uint_t                       naddrs;
    ngx_uint_t                       weight;
    ngx_uint_t                       max_fails;	//允许最大失败次数
    time_t                           fail_timeout;	//失败的时间区间

    unsigned                         down:1;	//服务器是否下线
    unsigned                         backup:1;	//服务器是否备份
} ngx_http_upstream_server_t;


#define NGX_HTTP_UPSTREAM_CREATE        0x0001
#define NGX_HTTP_UPSTREAM_WEIGHT        0x0002
#define NGX_HTTP_UPSTREAM_MAX_FAILS     0x0004
#define NGX_HTTP_UPSTREAM_FAIL_TIMEOUT  0x0008
#define NGX_HTTP_UPSTREAM_DOWN          0x0010
#define NGX_HTTP_UPSTREAM_BACKUP        0x0020


struct ngx_http_upstream_srv_conf_s {
	//控制load-balance机制,每一个upstream配置可对应一个ngx_http_upstream_srv_conf_s结构
	//也就是说每一个upstream块只能使用一种负载均衡算法
    ngx_http_upstream_peer_t         peer;
    void                           **srv_conf;

    ngx_array_t                     *servers;  /* ngx_http_upstream_server_t */

    ngx_uint_t                       flags;
    ngx_str_t                        host;
    u_char                          *file_name;
    ngx_uint_t                       line;
    in_port_t                        port;
    in_port_t                        default_port;
    ngx_uint_t                       no_port;  /* unsigned no_port:1 */

#if (NGX_HTTP_UPSTREAM_ZONE)
    ngx_shm_zone_t                  *shm_zone;
#endif
};


typedef struct {
    ngx_addr_t                      *addr;
    ngx_http_complex_value_t        *value;
} ngx_http_upstream_local_t;

/*
 * ngx_http_upstream_conf_t
 * 它用于设置upstream模块处理请求时的参数，包括连接，发送，接收的超时时间等。
 * */
typedef struct {
	//upstream框架的srv配置信息
    ngx_http_upstream_srv_conf_t    *upstream;
    //连接上游服务器的超时时间，单位为毫秒
    ngx_msec_t                       connect_timeout;
    //发送TCP包到上游服务器的超时时间，单位为毫秒，实际上就是写事件添加到定时器中时设置的超时时间
    ngx_msec_t                       send_timeout;
    //接收TCP包到上游服务器的超时时间，单位为毫秒
    ngx_msec_t                       read_timeout;
    ngx_msec_t                       timeout;
    ngx_msec_t                       next_upstream_timeout;
    //tcp的SO_SNOLOWAT选项，表示发送缓冲区的下限
    size_t                           send_lowat;
    /* 定义了接收头部的缓冲区分配的内存大小(ngx_http_upstream_t中的buffer缓冲区)，
     * 当不转发响应给下游或者在buffering标志位为0的情况下转发响应时，它同样表示
     * 接收包体的缓冲区大小
     * */
    size_t                           buffer_size;
    size_t                           limit_rate;

    size_t                           busy_buffers_size;
    size_t                           max_temp_file_size;
    //表示将缓冲区中的响应写入临时文件时一次写入字符流的最大长度
    size_t                           temp_file_write_size;

    size_t                           busy_buffers_size_conf;
    size_t                           max_temp_file_size_conf;
    size_t                           temp_file_write_size_conf;
    //以缓存响应方法转发上游服务器的包体时所使用的内存大小
    ngx_bufs_t                       bufs;

    ngx_uint_t                       ignore_headers;
    ngx_uint_t                       next_upstream;
    /* 在buffering标志位为1的情况下转发响应时，将有可能把响应存放到临时文件中，
     * 在ngx_http_upstream_t中的store标志位为1时，store_access表示所创建的目录，文件的权限
     * */
    ngx_uint_t                       store_access;
    ngx_uint_t                       next_upstream_tries;
    /* 决定转发响应方式的标志位，
     * buffering为1时表示打开缓存，这时认为上游的网速快于下游的网速，会尽量地在内存或者磁盘中缓存来自上游的响应，
     * buffering为0时，仅会开辟一块固定大小的内存块作为缓存来转发响应
     * */
    ngx_flag_t                       buffering;
    ngx_flag_t                       request_buffering;
    ngx_flag_t                       pass_request_headers;
    ngx_flag_t                       pass_request_body;
    //表示标志位,当它为1时,表示与上游服务器交互时将不检查Nginx与下游客户端间的连接是否断开
    //也就是说,即使下游客户端主动关闭了连接,也不会中断与上游服务器的交互
    ngx_flag_t                       ignore_client_abort;
    ngx_flag_t                       intercept_errors;
    ngx_flag_t                       cyclic_temp_file;
    ngx_flag_t                       force_ranges;

    ngx_path_t                      *temp_path;

    ngx_hash_t                       hide_headers_hash;
    /*
     * 当转发上游响应头部(ngx_http_upstream_t中headers_in结构体中的头部)给下游客户端时，
     * 如果不希望某些头部转发给下游，就设置到hide_headers动态数组中
     *
     * hide_headers是动态数组,实际上,upstream模块将会通过hide_headers来构造hide_headers_hash散列表
     * 由于upstream模块要求hide_headers不可以为空,所以必须要初始化hide_headers成员.upstream在解析完上游
     * 服务器返回的包头时，会调用ngx_http_upstream_process_headers方法按照hide_headers成员将本应转发给
     * 下游的一些http头部隐藏
     * upstream模块提供了ngx_http_upstream_hide_headers_hash方法来初始化hide_headers,但是仅可用在
     * 合并配置项方法内
     */
    ngx_array_t                     *hide_headers;
    /*
     * 当转发上游响应头部(ngx_http_upstream中headers_in结构体中的头部)给下游客户端时，upstream机制默认
     * 不会转发如“Date”，“Server”之类的头部，如果确实希望直接转发它们到下游，接设置到pass_headers动态数组中
     * */
    ngx_array_t                     *pass_headers;

    ngx_http_upstream_local_t       *local;

#if (NGX_HTTP_CACHE)
    ngx_shm_zone_t                  *cache_zone;
    ngx_http_complex_value_t        *cache_value;

    ngx_uint_t                       cache_min_uses;
    ngx_uint_t                       cache_use_stale;
    ngx_uint_t                       cache_methods;

    ngx_flag_t                       cache_lock;
    ngx_msec_t                       cache_lock_timeout;
    ngx_msec_t                       cache_lock_age;

    ngx_flag_t                       cache_revalidate;

    ngx_array_t                     *cache_valid;
    ngx_array_t                     *cache_bypass;
    ngx_array_t                     *no_cache;
#endif

    ngx_array_t                     *store_lengths;
    ngx_array_t                     *store_values;

#if (NGX_HTTP_CACHE)
    signed                           cache:2;
#endif
    signed                           store:2;
    unsigned                         intercept_404:1;
    unsigned                         change_buffering:1;

#if (NGX_HTTP_SSL)
    ngx_ssl_t                       *ssl;
    ngx_flag_t                       ssl_session_reuse;

    ngx_http_complex_value_t        *ssl_name;
    ngx_flag_t                       ssl_server_name;
    ngx_flag_t                       ssl_verify;
#endif

    ngx_str_t                        module;
} ngx_http_upstream_conf_t;


typedef struct {
    ngx_str_t                        name;
    ngx_http_header_handler_pt       handler;
    ngx_uint_t                       offset;
    ngx_http_header_handler_pt       copy_handler;
    ngx_uint_t                       conf;
    ngx_uint_t                       redirect;  /* unsigned   redirect:1; */
} ngx_http_upstream_header_t;


typedef struct {
    ngx_list_t                       headers;

    ngx_uint_t                       status_n;
    ngx_str_t                        status_line;

    ngx_table_elt_t                 *status;
    ngx_table_elt_t                 *date;
    ngx_table_elt_t                 *server;
    ngx_table_elt_t                 *connection;

    ngx_table_elt_t                 *expires;
    ngx_table_elt_t                 *etag;
    ngx_table_elt_t                 *x_accel_expires;
    ngx_table_elt_t                 *x_accel_redirect;
    ngx_table_elt_t                 *x_accel_limit_rate;

    ngx_table_elt_t                 *content_type;
    ngx_table_elt_t                 *content_length;

    ngx_table_elt_t                 *last_modified;
    ngx_table_elt_t                 *location;
    ngx_table_elt_t                 *accept_ranges;
    ngx_table_elt_t                 *www_authenticate;
    ngx_table_elt_t                 *transfer_encoding;
    ngx_table_elt_t                 *vary;

#if (NGX_HTTP_GZIP)
    ngx_table_elt_t                 *content_encoding;
#endif

    ngx_array_t                      cache_control;
    ngx_array_t                      cookies;

    off_t                            content_length_n;
    time_t                           last_modified_time;

    unsigned                         connection_close:1;
    unsigned                         chunked:1;
} ngx_http_upstream_headers_in_t;

//用于设置要访问的第三方服务器地址
typedef struct {
    ngx_str_t                        host;
    in_port_t                        port;
    ngx_uint_t                       no_port; /* unsigned no_port:1 */
    //地址个数
    ngx_uint_t                       naddrs;
    ngx_addr_t                      *addrs;
    //上游服务器的地址
    struct sockaddr                 *sockaddr;
    socklen_t                        socklen;

    ngx_resolver_ctx_t              *ctx;
} ngx_http_upstream_resolved_t;


typedef void (*ngx_http_upstream_handler_pt)(ngx_http_request_t *r,
    ngx_http_upstream_t *u);


struct ngx_http_upstream_s {
	//处理读事件的回调方法,每一个阶段都有不同的read_event_handler
    ngx_http_upstream_handler_pt     read_event_handler;
    //处理写事件的回调方法,每一个阶段都有不同的write_event_handler
    ngx_http_upstream_handler_pt     write_event_handler;
    //表示主动向上游服务器发起的连接
    ngx_peer_connection_t            peer;
    /*
     * 当向下游客户端转发响应时（ngx_http_request_t结构体中的subrequest_in_memory标志位为０）
     * 如果打开了缓存且认为上游网速更快（conf配置中的buffering标志位为１）,这时会使用pipe成员来
     * 转发响应。在使用这种方式转发响应时，必须由http模块在使用upstream机制前构造pipe结构体，否则
     * 会出现严重的coredump错误。
     * */
    ngx_event_pipe_t                *pipe;
    /*request_bufs 决定发送什么样的请求给上游服务器，在实现create_request方法时需要设置它*/
    ngx_chain_t                     *request_bufs;
    //定义了向下游发送响应的方式
    ngx_output_chain_ctx_t           output;
    ngx_chain_writer_ctx_t           writer;
    //上游的连接参数设置,包括连接，发送，接收的超时时间
    ngx_http_upstream_conf_t        *conf;
#if (NGX_HTTP_CACHE)
    ngx_array_t                     *caches;
#endif
    //上游的响应头
    /*
     * http模块在实现process_header方法时，如果希望upstream直接转发响应，就需要把解析出的响应
     * 头部适配为http响应头部，同时需要把包头中的信息设置到headers_in结构体中，这样会把headers_in中
     * 设置的头部添加到要发送到下游客户端的响应头部headers_out中
     * */
    ngx_http_upstream_headers_in_t   headers_in;
    /*通过resolved可以直接指定上游服务器地址*/
    ngx_http_upstream_resolved_t    *resolved;

    ngx_buf_t                        from_client;
    /*
     * buffer成员存储接收自上游服务器发来的相应内容，由于它会被复用，所以具有下列多种意义：
     * 1.在使用process_header方法解析上游响应的包头时，buffer中将会保存完整的响应包头；
     * 2.当下面的buffering成员为1，而且此时upstream是向下游转发上游的包体是，buffer没有意义;
     * 3.当buffering 标志位为0时，buffer缓存区会被用于反复地接收上游的包体，进而向下游转发；
     * 4.当upstream并不用于转发上游包体时，buffer会被用于反复接收上游的包体，HTTP模块实现的
     * 	 input_filter方法需要关注它
     * */
    ngx_buf_t                        buffer;
    //表示来自上游服务器的响应包体的长度
    off_t                            length;
    //从上游接收到的数据
    ngx_chain_t                     *out_bufs;
    //d
    ngx_chain_t                     *busy_bufs;
    ngx_chain_t                     *free_bufs;
    /* upstream 9种用户回调函数之一
     * 响应体数据也存放在buffer里,当ngx_http_request_t里的subrequest_in_memory标志位是1时,
     * upstream框架就会调用input_filter_init和input_filter这两个函数对响应体执行过滤处理,
     * 加工出对下游的有效数据
     * */
    ngx_int_t                      (*input_filter_init)(void *data);
    /*upstream 9种用户回调函数之一*/
    ngx_int_t                      (*input_filter)(void *data, ssize_t bytes);
    void                            *input_filter_ctx;

#if (NGX_HTTP_CACHE)
    ngx_int_t                      (*create_key)(ngx_http_request_t *r);
#endif
    /* upstream 9种用户回调函数之一(必须实现)
     * 在upstream机制启动时调用,生成能够与上游服务器正确通信的请求数据,
     * 例如http请求头,memchached命令,并把数据以ngx_chain_t的形式存放在
     * ngx_http_upstream_t/request_bufs中
     *
     * 生成发送到后端服务器的请求缓冲（缓冲链），在初始化upstream 时使用.*/
    ngx_int_t                      (*create_request)(ngx_http_request_t *r);
    /*
     * 在某台后端服务器出错的情况，nginx会尝试另一台后端服务器。
     * nginx选定新的服务器以后，会先调用此函数，
     * 以重新初始化upstream模块的工作状态，然后再次进行upstream连接。
     * */
    ngx_int_t                      (*reinit_request)(ngx_http_request_t *r);
    /* upstream 9种用户回调函数之一(必须实现)
     * 作用就是把数据分解出响应头和响应体两部分,并把响应头里的状态码
     * 内容长度等信息存储到headers_in和state成员,转化为下游可以理解的http格式
     *
     * 收到上游服务器的响应后就会回调process_header方法。但是由于框架异步接收,
     * 所以如果process_header返回NGX_AGAIN,那么是告诉upstream还没收到完整的响应包头，
     * 此时，对于本次upstream请求来说，再次接收到上游服务器发来的TCP流时，
     * 还会调用process_header方法处理，直到process_header函数返回非
     * NGX_AGAIN值，这一阶段才会停止.其后的数据都是响应体
     * */
    ngx_int_t                      (*process_header)(ngx_http_request_t *r);
    /*
     * 在客户端放弃请求时被调用。
     * 不需要在函数中实现关闭后端服务器连接的功能，系统会自动完成关闭连接的步骤，
     * 所以一般此函数不会进行任何具体工作。
     * */
    void                           (*abort_request)(ngx_http_request_t *r);
    /* upstream 9种用户回调函数之一(必须实现)
     * 销毁upstream请求时调用,正常完成与后端服务器的请求后调用该函数，
     * 与abort_request相同，一般也不会进行任何具体工作*/
    void                           (*finalize_request)(ngx_http_request_t *r,
                                         ngx_int_t rc);
    ngx_int_t                      (*rewrite_redirect)(ngx_http_request_t *r,
                                         ngx_table_elt_t *h, size_t prefix);
    ngx_int_t                      (*rewrite_cookie)(ngx_http_request_t *r,
                                         ngx_table_elt_t *h);

    ngx_msec_t                       timeout;
    //上游处理的状态信息
    ngx_http_upstream_state_t       *state;

    ngx_str_t                        method;
    ngx_str_t                        schema;
    ngx_str_t                        uri;

#if (NGX_HTTP_SSL)
    ngx_str_t                        ssl_name;
#endif

    ngx_http_cleanup_pt             *cleanup;

    unsigned                         store:1;
    unsigned                         cacheable:1;
    unsigned                         accel:1;
    unsigned                         ssl:1;
#if (NGX_HTTP_CACHE)
    unsigned                         cache_status:3;
#endif
    /*
     * 在向客户端转发上游服务器的包体时才有用，当buffering为1时，表示使用多个缓冲区
     * 以及磁盘文件来转发上游的响应包体。当nginx与上游间的网速远大与nginx与下游客户端间的网速时，
     * 让nginx开辟更多的内存甚至使用磁盘文件来缓存上游的响应包体，这是有意义的，他可以减轻上游
     * 服务器的并发能力。当buffering为0时，表示只使用上面的这一个buffer缓冲区来向下游转发响应包体
     * */
    unsigned                         buffering:1;
    unsigned                         keepalive:1;
    unsigned                         upgrade:1;
    /*
     * request_sent表示是否已经向上游服务器发送了请求,当request_sent为1时,表示upstream机制已经
     * 向上游服务器发送了全部或者部分的请求.事实上,这个标识位更多的是为了使用ngx_output_chain方法
     * 发送请求,因为该方法发送请求时会自动把为发送完的request_bufs链表记录下来,为了防止反复发送
     * 重复请求,必须有request_sent标志位记录是否调用过ngx_output_chain方法
     * */
    unsigned                         request_sent:1;
    /* 将上游服务器的响应划分为包头和包尾,如果把响应直接转发给客户端,header_sent标志位表示
     * 包头是否发送,为1时表示已经把包头转发给客户端了.如果不转发响应到客户端,则header_sent无意义
     * */
    unsigned                         header_sent:1;
};


typedef struct {
    ngx_uint_t                      status;
    ngx_uint_t                      mask;
} ngx_http_upstream_next_t;


typedef struct {
    ngx_str_t   key;
    ngx_str_t   value;
    ngx_uint_t  skip_empty;
} ngx_http_upstream_param_t;


ngx_int_t ngx_http_upstream_cookie_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
ngx_int_t ngx_http_upstream_header_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);

ngx_int_t ngx_http_upstream_create(ngx_http_request_t *r);
void ngx_http_upstream_init(ngx_http_request_t *r);
ngx_http_upstream_srv_conf_t *ngx_http_upstream_add(ngx_conf_t *cf,
    ngx_url_t *u, ngx_uint_t flags);
char *ngx_http_upstream_bind_set_slot(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
char *ngx_http_upstream_param_set_slot(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
ngx_int_t ngx_http_upstream_hide_headers_hash(ngx_conf_t *cf,
    ngx_http_upstream_conf_t *conf, ngx_http_upstream_conf_t *prev,
    ngx_str_t *default_hide_headers, ngx_hash_init_t *hash);


#define ngx_http_conf_upstream_srv_conf(uscf, module)                         \
    uscf->srv_conf[module.ctx_index]


extern ngx_module_t        ngx_http_upstream_module;
extern ngx_conf_bitmask_t  ngx_http_upstream_cache_method_mask[];
extern ngx_conf_bitmask_t  ngx_http_upstream_ignore_headers_masks[];


#endif /* _NGX_HTTP_UPSTREAM_H_INCLUDED_ */
