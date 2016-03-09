
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_SHMEM_H_INCLUDED_
#define _NGX_SHMEM_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>

/* 共享内存数据结构 */
typedef struct {
    u_char      *addr;/* 分配内存的起始地址 */
    size_t       size;//共享内存的长度
    ngx_str_t    name;//这块共享内存的名称,主要用作共享内存的唯一标识
    ngx_log_t   *log;//记录日志的ngx_log_t对象
    ngx_uint_t   exists;   /* unsigned  exists:1;  表示共享内存是否已经分配过的标志位 */
} ngx_shm_t;


ngx_int_t ngx_shm_alloc(ngx_shm_t *shm);
void ngx_shm_free(ngx_shm_t *shm);


#endif /* _NGX_SHMEM_H_INCLUDED_ */
