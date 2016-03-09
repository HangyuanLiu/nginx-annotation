
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_BUF_H_INCLUDED_
#define _NGX_BUF_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef void *            ngx_buf_tag_t;

typedef struct ngx_buf_s  ngx_buf_t;
/*
 * 缓冲区ngx_buf_t 是Nginx处理大数据的关键数据结构，它既应用与内存数据
 * 也应用于磁盘数据
 * */
struct ngx_buf_s {
	/* pos通常是用来告诉使用者本次应该从pos这个位置开始处理内存中的数据,这样设置是因为
	 * 同一个ngx_buf_t可能被多次反复处理.当然,pos的含义是由使用呀的模块定义的*/
    u_char          *pos;
    //last表示有效的内容到此为止,pos和last之间的内容,是希望nginx处理的内容
    u_char          *last;
    /* 处理文件时,file_pos和file_last的含义与处理内存时的last,pos相同*/
    off_t            file_pos;
    off_t            file_last;

    //如果ngx_buf_t缓冲区用于内存,那么start指向这段内存的起始位置(上界)
    u_char          *start;         /* start of buffer */
    //与start成员对应,指向缓冲区内存的末尾(下界)
    u_char          *end;           /* end of buffer */
    /*表示当前缓冲区的类型,例如由哪个模块使用就指向这个模块ngx_module_t变量的地址
     * void*类型指针，可以是任意数据*/
    ngx_buf_tag_t    tag;
    //引用的文件
    ngx_file_t      *file;

    ngx_buf_t       *shadow;


    /* the buf's content could be changed */
    //临时内存标志位,为1时表示数据在内存中且这段内存可以修改
    unsigned         temporary:1;

    /*
     * the buf's content is in a memory cache or in a read only memory
     * and must not be changed
     */
    unsigned         memory:1;

    /* the buf's content is mmap()ed and must not be changed */
    unsigned         mmap:1;
    //标志位,为1时表示可回收
    unsigned         recycled:1;
    //标志位,为1时表示这段缓冲区处理的是文件而不是内存
    unsigned         in_file:1;
    //标志位,为1时表示需要执行flush操作,立即输出本缓冲区
    unsigned         flush:1;
    /* 标志位,对于操作这块缓冲区时是否使用同步方式
     * */
    unsigned         sync:1;
    //为1表示当前是整个处理过程中的最后一块待处理的缓冲区,标志着http请求处理的结束
    unsigned         last_buf:1;
    //是当前数据块链(ngx_chain_t)里的最后一块,之后可能还会有数据需要处理
    unsigned         last_in_chain:1;

    unsigned         last_shadow:1;
    //标志位,表示当前缓冲区是否属于临时文件
    unsigned         temp_file:1;

    /* STUB */ int   num;
};

/* 在处理http请求时会经常创建多个缓冲区来存放数据,
 * Nginx把缓冲区块简单地组织为一个单向链表，在向用户发送http包体时，就要
 * 传入ngx_chain_t链表对象
 * */
struct ngx_chain_s {
    ngx_buf_t    *buf;	//指向当前的ngx_buf_t缓冲区
    /* 指向下一个ngx_chain_t,需要注意的是如果是最后一个ngx_chain_t,
     * 那么必须将next置为null,否则永远不会发送成功,而且这个请求将一直不会结束
     * */
    ngx_chain_t  *next;
};


typedef struct {
    ngx_int_t    num;
    size_t       size;
} ngx_bufs_t;


typedef struct ngx_output_chain_ctx_s  ngx_output_chain_ctx_t;

typedef ngx_int_t (*ngx_output_chain_filter_pt)(void *ctx, ngx_chain_t *in);

#if (NGX_HAVE_FILE_AIO)
typedef void (*ngx_output_chain_aio_pt)(ngx_output_chain_ctx_t *ctx,
    ngx_file_t *file);
#endif

struct ngx_output_chain_ctx_s {
    ngx_buf_t                   *buf;
    ngx_chain_t                 *in;
    ngx_chain_t                 *free;
    ngx_chain_t                 *busy;

    unsigned                     sendfile:1;
    unsigned                     directio:1;
#if (NGX_HAVE_ALIGNED_DIRECTIO)
    unsigned                     unaligned:1;
#endif
    unsigned                     need_in_memory:1;
    unsigned                     need_in_temp:1;
#if (NGX_HAVE_FILE_AIO || NGX_THREADS)
    unsigned                     aio:1;
#endif

#if (NGX_HAVE_FILE_AIO)
    ngx_output_chain_aio_pt      aio_handler;
#if (NGX_HAVE_AIO_SENDFILE)
    ssize_t                    (*aio_preload)(ngx_buf_t *file);
#endif
#endif

#if (NGX_THREADS)
    ngx_int_t                  (*thread_handler)(ngx_thread_task_t *task,
                                                 ngx_file_t *file);
    ngx_thread_task_t           *thread_task;
#endif

    off_t                        alignment;

    ngx_pool_t                  *pool;
    ngx_int_t                    allocated;
    ngx_bufs_t                   bufs;
    ngx_buf_tag_t                tag;

    ngx_output_chain_filter_pt   output_filter;
    void                        *filter_ctx;
};


typedef struct {
    ngx_chain_t                 *out;
    ngx_chain_t                **last;
    ngx_connection_t            *connection;
    ngx_pool_t                  *pool;
    off_t                        limit;
} ngx_chain_writer_ctx_t;


#define NGX_CHAIN_ERROR     (ngx_chain_t *) NGX_ERROR

//确定缓冲区是否在内存里
#define ngx_buf_in_memory(b)        (b->temporary || b->memory || b->mmap)
#define ngx_buf_in_memory_only(b)   (ngx_buf_in_memory(b) && !b->in_file)

#define ngx_buf_special(b)                                                   \
    ((b->flush || b->last_buf || b->sync)                                    \
     && !ngx_buf_in_memory(b) && !b->in_file)

#define ngx_buf_sync_only(b)                                                 \
    (b->sync                                                                 \
     && !ngx_buf_in_memory(b) && !b->in_file && !b->flush && !b->last_buf)

#define ngx_buf_size(b)                                                      \
    (ngx_buf_in_memory(b) ? (off_t) (b->last - b->pos):                      \
                            (b->file_last - b->file_pos))

ngx_buf_t *ngx_create_temp_buf(ngx_pool_t *pool, size_t size);
ngx_chain_t *ngx_create_chain_of_bufs(ngx_pool_t *pool, ngx_bufs_t *bufs);


#define ngx_alloc_buf(pool)  ngx_palloc(pool, sizeof(ngx_buf_t))
#define ngx_calloc_buf(pool) ngx_pcalloc(pool, sizeof(ngx_buf_t))

ngx_chain_t *ngx_alloc_chain_link(ngx_pool_t *pool);
#define ngx_free_chain(pool, cl)                                             \
    cl->next = pool->chain;                                                  \
    pool->chain = cl



ngx_int_t ngx_output_chain(ngx_output_chain_ctx_t *ctx, ngx_chain_t *in);
ngx_int_t ngx_chain_writer(void *ctx, ngx_chain_t *in);

ngx_int_t ngx_chain_add_copy(ngx_pool_t *pool, ngx_chain_t **chain,
    ngx_chain_t *in);
ngx_chain_t *ngx_chain_get_free_buf(ngx_pool_t *p, ngx_chain_t **free);
void ngx_chain_update_chains(ngx_pool_t *p, ngx_chain_t **free,
    ngx_chain_t **busy, ngx_chain_t **out, ngx_buf_tag_t tag);

off_t ngx_chain_coalesce_file(ngx_chain_t **in, off_t limit);

ngx_chain_t *ngx_chain_update_sent(ngx_chain_t *in, off_t sent);

#endif /* _NGX_BUF_H_INCLUDED_ */
