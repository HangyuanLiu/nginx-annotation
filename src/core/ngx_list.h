
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_LIST_H_INCLUDED_
#define _NGX_LIST_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_list_part_s  ngx_list_part_t;
//描述链表的一个元素
struct ngx_list_part_s {
    void             *elts;	//指向数组的起始地址
    ngx_uint_t        nelts;//表示数组中已经使用了多少个元素。nelts必须小于ngx_list_t结构体中的nalloc
    ngx_list_part_t  *next;
};

//描述整个链表
typedef struct {
    ngx_list_part_t  *last;	//指向链表的最后一个数组元素
    ngx_list_part_t   part;	//链表的首个数组元素(非指针)
    size_t            size;	//用户要存储的一个数据所占用的字节数必须小于或等于size
    //链表的数组元素一旦分配后是不可更改的,nalloc表示每个ngx_list_part_t数组的容量,即最多可存储多少个数据
    ngx_uint_t        nalloc;
    ngx_pool_t       *pool;
} ngx_list_t;

//创建链表,n代表链表数组中,每个数组可以存储几个元素(相当于nalloc),size则为ngx_list_t中的size
ngx_list_t *ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size);

//初始化一个已有的链表，被ngx_list_create调用
static ngx_inline ngx_int_t
ngx_list_init(ngx_list_t *list, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    list->part.elts = ngx_palloc(pool, n * size);
    if (list->part.elts == NULL) {
        return NGX_ERROR;
    }

    list->part.nelts = 0;
    list->part.next = NULL;
    list->last = &list->part;
    list->size = size;
    list->nalloc = n;
    list->pool = pool;

    return NGX_OK;
}


/*
 *
 *  the iteration through the list:
 *
 *  part = &list.part;
 *  data = part->elts;
 *
 *  for (i = 0 ;; i++) {
 *
 *      if (i >= part->nelts) {
 *          if (part->next == NULL) {
 *              break;
 *          }
 *
 *          part = part->next;
 *          data = part->elts;
 *          i = 0;
 *      }
 *
 *      ...  data[i] ...
 *
 *  }
 */


void *ngx_list_push(ngx_list_t *list);


#endif /* _NGX_LIST_H_INCLUDED_ */
