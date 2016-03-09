
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CONNECTION_H_INCLUDED_
#define _NGX_CONNECTION_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_listening_s  ngx_listening_t;

struct ngx_listening_s {
	//socket套接字句柄
    ngx_socket_t        fd;
    //监听sockaddr地址
    struct sockaddr    *sockaddr;
    //sockaddr地址长度
    socklen_t           socklen;    /* size of sockaddr */
    //存储IP地址的字符串addr_text最大长度，即它指定了addr_text所分配的内存大小
    size_t              addr_text_max_len;
    //以字符串形式存储IP地址
    ngx_str_t           addr_text;
    //套接字类型。例如，当type是SOCK_STREAM时，表示tcp
    int                 type;
    /*
     * TCP实现监听是的backlog队列，它表示允许正在通过三次握手建立TCP连接
     * 但还没有任何进程开始处理的连接最大数
     * */
    int                 backlog;
    //内核中对于这个套接字的接收缓存区大小
    int                 rcvbuf;
    //内核中对于这个套接字的发送缓冲区大小
    int                 sndbuf;
#if (NGX_HAVE_KEEPALIVE_TUNABLE)
    int                 keepidle;
    int                 keepintvl;
    int                 keepcnt;
#endif

    /* handler of accepted connection */
    //当新的TCP连接成功建立后的处理方法
    ngx_connection_handler_pt   handler;
    /*
     * 实际上框架并不使用servers指针，它更多是作为一个保留指针，
     * 目前主要用于http或者mail等模块，用于保存当前监听端口对应着的所有主机名
     * */
    void               *servers;  /* array of ngx_http_in_addr_t, for example */
    //log和logp都是可用的日志对象的指针
    ngx_log_t           log;
    ngx_log_t          *logp;
    //如果为新的tcp连接创建内存池，则内存池的初始大小应该是pool_size
    size_t              pool_size;
    /* should be here because of the AcceptEx() preread */
    size_t              post_accept_buffer_size;
    /* should be here because of the deferred accept */
    /*
     * TCP_DEFER_ACCEPTE选项将在建立tcp连接成功且接收到用户的请求数据后，
     * 才向对监听套接字感兴趣的进程发送事件通知，而连接建立成功后，如果
     * post_accept_timeout秒后仍然没有收到的用户数据，则内核直接丢弃连接
     * */
    ngx_msec_t          post_accept_timeout;
    //前一个ngx_listening_t结构，多个结构体之间由previous指针组成单链表
    ngx_listening_t    *previous;
    //当前监听句柄对应着的ngx_connection_t结构体
    ngx_connection_t   *connection;
    /*
     * 标志位，为1则表示在当前监听句柄有效，且执行ngx_init_cycle时不关闭监听端口，
     * 为0时则正常关闭。该标志位框架代码会自动设置
     * */
    unsigned            open:1;
    /*
     * 标志位，为1表示使用已有的ngx_cycle_t来初始化新的ngx_cycle_t结构体时，
     * 不关闭原先打开的监听端口，这对运行中升级程序很有用，remain为0时，
     * 表示正常关闭曾经打开的监听端口。该标志位框架代码会自动设置，参见ngx_init_cycle
     * */
    unsigned            remain:1;
    /*
     * 标志位，为1时表示跳过设置当前ngx_listening_t结构体中的套接字，
     * 为0时正常初始化套接字。该标志位框架代码会自动设置
     * */
    unsigned            ignore:1;

    unsigned            bound:1;       /* already bound */
    unsigned            inherited:1;   /* inherited from previous process */
    unsigned            nonblocking_accept:1;
    //标志位，为1时表示当前结构体对应的套接字已经监听
    unsigned            listen:1;
    //表示套接字是否阻塞，目前该标志位没有意义
    unsigned            nonblocking:1;
    unsigned            shared:1;    /* shared between threads or processes */
    //标志位。为1时表示nginx会将网络地址转变为字符串形式的地址
    unsigned            addr_ntop:1;

#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
    unsigned            ipv6only:1;
#endif
    unsigned            keepalive:2;

#if (NGX_HAVE_DEFERRED_ACCEPT)
    unsigned            deferred_accept:1;
    unsigned            delete_deferred:1;
    unsigned            add_deferred:1;
#ifdef SO_ACCEPTFILTER
    char               *accept_filter;
#endif
#endif
#if (NGX_HAVE_SETFIB)
    int                 setfib;
#endif

#if (NGX_HAVE_TCP_FASTOPEN)
    int                 fastopen;
#endif

};


typedef enum {
     NGX_ERROR_ALERT = 0,
     NGX_ERROR_ERR,
     NGX_ERROR_INFO,
     NGX_ERROR_IGNORE_ECONNRESET,
     NGX_ERROR_IGNORE_EINVAL
} ngx_connection_log_error_e;


typedef enum {
     NGX_TCP_NODELAY_UNSET = 0,
     NGX_TCP_NODELAY_SET,
     NGX_TCP_NODELAY_DISABLED
} ngx_connection_tcp_nodelay_e;


typedef enum {
     NGX_TCP_NOPUSH_UNSET = 0,
     NGX_TCP_NOPUSH_SET,
     NGX_TCP_NOPUSH_DISABLED
} ngx_connection_tcp_nopush_e;


#define NGX_LOWLEVEL_BUFFERED  0x0f
#define NGX_SSL_BUFFERED       0x01
#define NGX_SPDY_BUFFERED      0x02


struct ngx_connection_s {
	/* 连接未使用时,data成员用于充当连接池中空闲连接链表中的next指针.当连接被使用时,data的意义由使用它的Nginx模块而定
	 * 如在HTTP框架中,data指向ngx_http_request_t请求
	 * */
    void               *data;
    ngx_event_t        *read;//连接对应的读事件
    ngx_event_t        *write;//连接对应的写事件
    //套接字句柄
    ngx_socket_t        fd;
    //直接接收网络字符流的方法
    ngx_recv_pt         recv;
    //直接发送网络字符流的方法
    ngx_send_pt         send;
    //以ngx_chain_t链表为参数来接收网络字符流的方法
    ngx_recv_chain_pt   recv_chain;
    ngx_send_chain_pt   send_chain;
    //这个连接对应的ngx_listening_t监听对象，此连接有listening监听端口的事件建立
    ngx_listening_t    *listening;
    //这个连接上已经发送出去的字节数
    off_t               sent;
    //可以记录日志的ngx_log_t对象
    ngx_log_t          *log;
    /*
     * 内存池。一般在accept一个连接时，会创建一个内存池，而在这个连接结束时会销毁内存池。
     * 注意，这里所说的连接是指成功建立的tcp连接，所有的ngx_connection_t结构体都是预
     * 分配的。这个内存池的大小将由上面的listening监听对象中的pool_size成员决定。
     * */
    ngx_pool_t         *pool;
    //连接客户端的结构体
    struct sockaddr    *sockaddr;
    socklen_t           socklen;
    //连接客户端字符串形式的ip地址
    ngx_str_t           addr_text;

    ngx_str_t           proxy_protocol_addr;

#if (NGX_SSL)
    ngx_ssl_connection_t  *ssl;
#endif
    //本机的监听端口对应的sockaddr结构体，也就是listening监听对象中的sockaddr成员
    struct sockaddr    *local_sockaddr;
    socklen_t           local_socklen;
    /* 用于接收，缓存客户端发来的字符流，每个事件消费模块可自由决定从连接池中分配多大的
     * 空间给buffer这个接收缓存字段。例如，在http模块中，它的大小决定与client_header_buffer_size配置项
    */
    ngx_buf_t          *buffer;
    /*
     * 该字段用来将当前连接以双向链表元素的形式添加到ngx_cycle_t核心结构体的reusable_connections_queue
     * 双向链表中，表示可以重用的连接
     * */
    ngx_queue_t         queue;
    /* 连接使用次数，ngx_connection_t结构体每次建立一条来自客户端的连接，或者用于主动向后端
     * 服务器发起连接时(ngx_peer_connection也使用它)，number都会加1
     * */
    ngx_atomic_uint_t   number;
    //处理的请求次数
    ngx_uint_t          requests;

    unsigned            buffered:8;

    unsigned            log_error:3;     /* ngx_connection_log_error_e */
    //标志位，为1是表示不期待字符流结束
    unsigned            unexpected_eof:1;
    //标志位，为1时表示已经超时
    unsigned            timedout:1;
    unsigned            error:1;
    /*
     * 标志位，为1时表示连接已经销毁。这里的连接指的是tcp连接，而不是ngx_connection_t结构体
     * 当destroyed为1时，ngx_connection_t结构体仍然存在，但其对应的套接字，内存池已经不可用
     * */
    unsigned            destroyed:1;
    //标志位，为1时表示连接处于空闲状态，如keepalive请求中两次请求之间的状态
    unsigned            idle:1;
    //标志位，为1时表示连接可重用，它与上面的queue字段是对应使用的
    unsigned            reusable:1;
    //标志位，表示连接关闭
    unsigned            close:1;
    //标志位，为1时表示正在将文件中的数据发往连接的另一端
    unsigned            sendfile:1;
    unsigned            sndlowat:1;
    //标志位，表示如何使用tcp的nodelay特性
    unsigned            tcp_nodelay:2;   /* ngx_connection_tcp_nodelay_e */
    //标志位，表示如何使用tcp的nopush特性
    unsigned            tcp_nopush:2;    /* ngx_connection_tcp_nopush_e */

    unsigned            need_last_buf:1;

#if (NGX_HAVE_IOCP)
    unsigned            accept_context_updated:1;
#endif

#if (NGX_HAVE_AIO_SENDFILE)
    unsigned            busy_count:2;
#endif

#if (NGX_THREADS)
    ngx_thread_task_t  *sendfile_task;
#endif
};


#define ngx_set_connection_log(c, l)                                         \
                                                                             \
    c->log->file = l->file;                                                  \
    c->log->next = l->next;                                                  \
    c->log->writer = l->writer;                                              \
    c->log->wdata = l->wdata;                                                \
    if (!(c->log->log_level & NGX_LOG_DEBUG_CONNECTION)) {                   \
        c->log->log_level = l->log_level;                                    \
    }


ngx_listening_t *ngx_create_listening(ngx_conf_t *cf, void *sockaddr,
    socklen_t socklen);
ngx_int_t ngx_set_inherited_sockets(ngx_cycle_t *cycle);
ngx_int_t ngx_open_listening_sockets(ngx_cycle_t *cycle);
void ngx_configure_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_connection(ngx_connection_t *c);
ngx_int_t ngx_connection_local_sockaddr(ngx_connection_t *c, ngx_str_t *s,
    ngx_uint_t port);
ngx_int_t ngx_connection_error(ngx_connection_t *c, ngx_err_t err, char *text);

ngx_connection_t *ngx_get_connection(ngx_socket_t s, ngx_log_t *log);
void ngx_free_connection(ngx_connection_t *c);

void ngx_reusable_connection(ngx_connection_t *c, ngx_uint_t reusable);

#endif /* _NGX_CONNECTION_H_INCLUDED_ */
