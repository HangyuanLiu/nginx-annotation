
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_SLAB_H_INCLUDED_
#define _NGX_SLAB_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_slab_page_s  ngx_slab_page_t;

struct ngx_slab_page_s {
	/*
	 * slab为使用较为复杂的一个字段，有以下四种使用情况
	 * a.存储为些结构相连的pages的数目(slab page管理结构)
	 * b.存储标记slot块使用情况的bitmap(size = exact_size)
	 * c.存储slot块的大小(size < exact_size)
	 * d.存储标记chunk的使用情况及chunk大小(size > exact_size)
	 * */
    uintptr_t         slab; //保存当前页的信息,每个bit位代表一个slot块的状态.1为使用,0为空闲
    ngx_slab_page_t  *next; //下一个
    uintptr_t         prev; //上一个
};


typedef struct {
	//为下面的互斥锁成员ngx_shmtx_t mutex服务，使用信号量作进程同步工具时会用到它
    ngx_shmtx_sh_t    lock;

    size_t            min_size;//最小分配单元
    size_t            min_shift;//最小分配单元对应的位移
    /* 每一页对应一个ngx_slab_page_t页描述结构体，所有的ngx_slab_page_t存放在
     * 连续的内存中构成数组，而pages就是数组首地址
     * */
    ngx_slab_page_t  *pages;//页数组,指向slab page管理结构的开始位置。
    ngx_slab_page_t  *last;
    ngx_slab_page_t   free;//空闲页链表

    u_char           *start;//可分配空间pages数组的起始地址
    u_char           *end;//内存块的结束地址

    ngx_shmtx_t       mutex;

    u_char           *log_ctx;
    u_char            zero;

    unsigned          log_nomem:1;
    //由各个使用slab的模块自由使用，slab管理内存时不会用到它
    void             *data;
    void             *addr;
} ngx_slab_pool_t;

//初始化slab池
void ngx_slab_init(ngx_slab_pool_t *pool);
void *ngx_slab_alloc(ngx_slab_pool_t *pool, size_t size);
void *ngx_slab_alloc_locked(ngx_slab_pool_t *pool, size_t size);
void *ngx_slab_calloc(ngx_slab_pool_t *pool, size_t size);
void *ngx_slab_calloc_locked(ngx_slab_pool_t *pool, size_t size);
void ngx_slab_free(ngx_slab_pool_t *pool, void *p);
//释放空间
void ngx_slab_free_locked(ngx_slab_pool_t *pool, void *p);


#endif /* _NGX_SLAB_H_INCLUDED_ */
