
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CONF_FILE_H_INCLUDED_
#define _NGX_CONF_FILE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/*
 *        AAAA  number of arguments
 *      FF      command flags
 *    TT        command type, i.e. HTTP "location" or "server" command
 */

#define NGX_CONF_NOARGS      0x00000001	//不携带任何参数
#define NGX_CONF_TAKE1       0x00000002	//必须携带一个参数
#define NGX_CONF_TAKE2       0x00000004
#define NGX_CONF_TAKE3       0x00000008
#define NGX_CONF_TAKE4       0x00000010
#define NGX_CONF_TAKE5       0x00000020
#define NGX_CONF_TAKE6       0x00000040
#define NGX_CONF_TAKE7       0x00000080

#define NGX_CONF_MAX_ARGS    8

#define NGX_CONF_TAKE12      (NGX_CONF_TAKE1|NGX_CONF_TAKE2)
#define NGX_CONF_TAKE13      (NGX_CONF_TAKE1|NGX_CONF_TAKE3)

#define NGX_CONF_TAKE23      (NGX_CONF_TAKE2|NGX_CONF_TAKE3)

#define NGX_CONF_TAKE123     (NGX_CONF_TAKE1|NGX_CONF_TAKE2|NGX_CONF_TAKE3)
#define NGX_CONF_TAKE1234    (NGX_CONF_TAKE1|NGX_CONF_TAKE2|NGX_CONF_TAKE3   \
                              |NGX_CONF_TAKE4)

#define NGX_CONF_ARGS_NUMBER 0x000000ff
#define NGX_CONF_BLOCK       0x00000100
#define NGX_CONF_FLAG        0x00000200	//配置项携带的参数只能是1个，并且参数的值只能是on|off
#define NGX_CONF_ANY         0x00000400	//不验证配置项携带的参数个数
#define NGX_CONF_1MORE       0x00000800	//配置项携带的参数个数必须超过1个
#define NGX_CONF_2MORE       0x00001000	//超过2个
#define NGX_CONF_MULTI       0x00000000  /* compatibility */

#define NGX_DIRECT_CONF      0x00010000

#define NGX_MAIN_CONF        0x01000000	//配置项可以出现在全局配置中，不属于任何{}配置块
#define NGX_ANY_CONF         0x0F000000


/* Nginx 采用-1 表示未初始化，由于C是强类型语言
 * 单纯的整数-1不能直接与其它类型比较，需要做类型转换
 * */
#define NGX_CONF_UNSET       -1
#define NGX_CONF_UNSET_UINT  (ngx_uint_t) -1
#define NGX_CONF_UNSET_PTR   (void *) -1
#define NGX_CONF_UNSET_SIZE  (size_t) -1
#define NGX_CONF_UNSET_MSEC  (ngx_msec_t) -1


#define NGX_CONF_OK          NULL
#define NGX_CONF_ERROR       (void *) -1

#define NGX_CONF_BLOCK_START 1
#define NGX_CONF_BLOCK_DONE  2
#define NGX_CONF_FILE_DONE   3

#define NGX_CORE_MODULE      0x45524F43  /* "CORE" */
#define NGX_CONF_MODULE      0x464E4F43  /* "CONF" */


#define NGX_MAX_CONF_ERRSTR  1024

/*
 * 配置项解析的单位条目数据结构,
 * 一个ngx_command代表一个配置项，
 * 多条自定义的配置项存储在一个数组中
 * */
struct ngx_command_s {
    ngx_str_t             name;//配置项名称
    ngx_uint_t            type;//配置项指令属性集合，type决定这个配置项可以在哪些块中出现，以及可以携带的参数类型和个数
    char               *(*set)(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);//回调函数，nginx提供了14个现成的，也可以自己写
    ngx_uint_t            conf;//指示配置项所处内存的相对偏移位置，在type中没有设置NGX_DIRECT_CONF和NGX_MAIN_CONF时生效
    ngx_uint_t            offset;//转换后控制值的存放位置（offset表示当前配置项在整个内存配置项的结构体中的偏移位置,以字节为单位）
    void                 *post;	//配置项读取后的处理方法，必须是ngx_conf_post_t结构的指针
};

#define ngx_null_command  { ngx_null_string, 0, NULL, 0, 0, NULL }


struct ngx_open_file_s {
    ngx_fd_t              fd;
    ngx_str_t             name;

    void                (*flush)(ngx_open_file_t *file, ngx_log_t *log);
    void                 *data;
};


#define NGX_MODULE_V1          0, 0, 0, 0, 0, 0, 1
#define NGX_MODULE_V1_PADDING  0, 0, 0, 0, 0, 0, 0, 0

//用来说明这个模块本身的信息,包含配置信息,上下文信息
struct  ngx_module_s{
	//下面的七个变量不需要在定义时赋值,一般用Nginx准备好的宏NGX_MODULE_V1来定义

	/* 当前模块在同类模块中的序号,这个成员常常是由管理这类模块的一个Nginx核心模块设置的
	 * 对于所有http模块而言,ctx_index是由核心模块ngx_http_module设置的
	 * Nginx的模块化设计非常依赖于各个模块的顺序,它们既用于表达式优先级,也用于表明各个
	 * 模块的位置,借以帮助Nginx框架快速获得某个模块的数据
	 * */
    ngx_uint_t            ctx_index;
    /*表示当前模块在ngx_modules数组中的序号，表示在所有模块中的序号
     * Nginx启动时会根据ngx_modules数组设置各模块的index值*/
    ngx_uint_t            index;

    ngx_uint_t            spare0;
    ngx_uint_t            spare1;
    ngx_uint_t            spare2;
    ngx_uint_t            spare3;

    ngx_uint_t            version;//当前模块版本号,目前只有一种,默认为1
    /*
     * ctx用于指向一类模块的上下文结构体.
     * nginx模块有许多种类,不同类模块之间的功能差别很大.
     * 所以,每个模块都有了自己的特性,而ctx将会指向特定类型模块的公共接口.
     * 例如,HTTP模块中,ctx需要指向ngx_http_module_t结构体
     * */
    void                 *ctx;//指向当前模块特有数据类型
    ngx_command_t        *commands;//当前模块配置项的解析数组
    /*
     * type 模块类型,它与ctx指针是紧密相关的
     * NGX_CORE_MODULE
     * NGX_EVENT_MODULE
     * NGX_CONF_MODULE
     * NGX_HTTP_MODULE
     * NGX_MAIL_MODULE
     * */
    ngx_uint_t            type;
    /*
     * 在Nginx的启动,停止过程中,以下7个函数指针表示有7个执行点会分别调用
     * 这7种方法.不需要则置为NULL
     * */
    //不使用
    ngx_int_t           (*init_master)(ngx_log_t *log);
    //init_module回调方法在初始化所有模块时(ngx_init_cycle函数中)被调用,在worker/master模式下,这个阶段会在启动worker子进程前完成
    ngx_int_t           (*init_module)(ngx_cycle_t *cycle);
    /*
     * init_process回调方法在正常服务前被调用.在master/worker模式下,
     * 多个worker子进程已经产生,在每个worker进程的初始化过程会调用
     * 所有模块的init_precess函数
     * */
    ngx_int_t           (*init_process)(ngx_cycle_t *cycle);
    ngx_int_t           (*init_thread)(ngx_cycle_t *cycle);
    void                (*exit_thread)(ngx_cycle_t *cycle);
    //在worker进程退出钱调用
    void                (*exit_process)(ngx_cycle_t *cycle);
    //在master进程退出前被调用
    void                (*exit_master)(ngx_cycle_t *cycle);
    //8个保留字段,一般用NGX_MODULE_V1_PADDING来填充
    uintptr_t             spare_hook0;
    uintptr_t             spare_hook1;
    uintptr_t             spare_hook2;
    uintptr_t             spare_hook3;
    uintptr_t             spare_hook4;
    uintptr_t             spare_hook5;
    uintptr_t             spare_hook6;
    uintptr_t             spare_hook7;
} ;

//core核心模块的具体化ctx上下文
typedef struct {
	//核心模块名称
    ngx_str_t             name;
    //解析配置项前，nginx框架会调用create_conf方法
    void               *(*create_conf)(ngx_cycle_t *cycle);
    //解析配置项完成后，nginx框架会调用init_conf方法
    char               *(*init_conf)(ngx_cycle_t *cycle, void *conf);
} ngx_core_module_t;


typedef struct {
    ngx_file_t            file;
    ngx_buf_t            *buffer;	//存放待解析的token字的缓冲区，类似于编译器中的词法缓冲区
    ngx_uint_t            line;
} ngx_conf_file_t;


typedef char *(*ngx_conf_handler_pt)(ngx_conf_t *cf,
    ngx_command_t *dummy, void *conf);

/*
 * 表示解析当前配置指令时的运行环境数据
 * 因为Nginx的配置文件是有层次的，分成main/http/server/location等作用域，
 * 不同的域里指令的含义和处理方式都有可能不同，所以nginx在解析配置文件时必须
 * 使用ngx_conf_t来保存当前的基本信息，进入或退出一个配置块都会变更ngx_conf_t(该结构体会在不同函数间传递)
 * 指令的解析必须要参考ngx_conf_t环境数据才能正确处理
 * */
struct ngx_conf_s {
    char                 *name;	//暂时未使用
    ngx_array_t          *args;	//配置文件里的指令字符串数组

    ngx_cycle_t          *cycle;
    ngx_pool_t           *pool;	//内存池对象
    ngx_pool_t           *temp_pool;
    ngx_conf_file_t      *conf_file;
    ngx_log_t            *log;	//日志对象
    //当前环境，指示了当前解析所需参考的环境数据，相当于一个临时变量，判断当前的ctx的状态，对于http模块，为ngx_http_conf_ctx_t
    void                 *ctx;
    ngx_uint_t            module_type;
    ngx_uint_t            cmd_type;	//当前的命令类型

    ngx_conf_handler_pt   handler;
    char                 *handler_conf;
};


typedef char *(*ngx_conf_post_handler_pt) (ngx_conf_t *cf,
    void *data, void *conf);

typedef struct {
    ngx_conf_post_handler_pt  post_handler;
} ngx_conf_post_t;


typedef struct {
    ngx_conf_post_handler_pt  post_handler;
    char                     *old_name;
    char                     *new_name;
} ngx_conf_deprecated_t;


typedef struct {
    ngx_conf_post_handler_pt  post_handler;
    ngx_int_t                 low;
    ngx_int_t                 high;
} ngx_conf_num_bounds_t;


typedef struct {
    ngx_str_t                 name;
    ngx_uint_t                value;
} ngx_conf_enum_t;


#define NGX_CONF_BITMASK_SET  1

typedef struct {
    ngx_str_t                 name;
    ngx_uint_t                mask;
} ngx_conf_bitmask_t;



char * ngx_conf_deprecated(ngx_conf_t *cf, void *post, void *data);
char *ngx_conf_check_num_bounds(ngx_conf_t *cf, void *post, void *data);


#define ngx_get_conf(conf_ctx, module)  conf_ctx[module.index]



#define ngx_conf_init_value(conf, default)                                   \
    if (conf == NGX_CONF_UNSET) {                                            \
        conf = default;                                                      \
    }

#define ngx_conf_init_ptr_value(conf, default)                               \
    if (conf == NGX_CONF_UNSET_PTR) {                                        \
        conf = default;                                                      \
    }

#define ngx_conf_init_uint_value(conf, default)                              \
    if (conf == NGX_CONF_UNSET_UINT) {                                       \
        conf = default;                                                      \
    }

#define ngx_conf_init_size_value(conf, default)                              \
    if (conf == NGX_CONF_UNSET_SIZE) {                                       \
        conf = default;                                                      \
    }

#define ngx_conf_init_msec_value(conf, default)                              \
    if (conf == NGX_CONF_UNSET_MSEC) {                                       \
        conf = default;                                                      \
    }

#define ngx_conf_merge_value(conf, prev, default)                            \
    if (conf == NGX_CONF_UNSET) {                                            \
        conf = (prev == NGX_CONF_UNSET) ? default : prev;                    \
    }

#define ngx_conf_merge_ptr_value(conf, prev, default)                        \
    if (conf == NGX_CONF_UNSET_PTR) {                                        \
        conf = (prev == NGX_CONF_UNSET_PTR) ? default : prev;                \
    }

#define ngx_conf_merge_uint_value(conf, prev, default)                       \
    if (conf == NGX_CONF_UNSET_UINT) {                                       \
        conf = (prev == NGX_CONF_UNSET_UINT) ? default : prev;               \
    }

#define ngx_conf_merge_msec_value(conf, prev, default)                       \
    if (conf == NGX_CONF_UNSET_MSEC) {                                       \
        conf = (prev == NGX_CONF_UNSET_MSEC) ? default : prev;               \
    }

#define ngx_conf_merge_sec_value(conf, prev, default)                        \
    if (conf == NGX_CONF_UNSET) {                                            \
        conf = (prev == NGX_CONF_UNSET) ? default : prev;                    \
    }

#define ngx_conf_merge_size_value(conf, prev, default)                       \
    if (conf == NGX_CONF_UNSET_SIZE) {                                       \
        conf = (prev == NGX_CONF_UNSET_SIZE) ? default : prev;               \
    }

#define ngx_conf_merge_off_value(conf, prev, default)                        \
    if (conf == NGX_CONF_UNSET) {                                            \
        conf = (prev == NGX_CONF_UNSET) ? default : prev;                    \
    }

#define ngx_conf_merge_str_value(conf, prev, default)                        \
	if (conf.data == NULL) {                                                 \
        if (prev.data) {                                                     \
            conf.len = prev.len;                                             \
            conf.data = prev.data;                                           \
        } else {                                                             \
            conf.len = sizeof(default) - 1;                                  \
            conf.data = (u_char *) default;                                  \
        }                                                                    \
    }

#define ngx_conf_merge_bufs_value(conf, prev, default_num, default_size)     \
    if (conf.num == 0) {                                                     \
        if (prev.num) {                                                      \
            conf.num = prev.num;                                             \
            conf.size = prev.size;                                           \
        } else {                                                             \
            conf.num = default_num;                                          \
            conf.size = default_size;                                        \
        }                                                                    \
    }

#define ngx_conf_merge_bitmask_value(conf, prev, default)                    \
    if (conf == 0) {                                                         \
        conf = (prev == 0) ? default : prev;                                 \
    }


char *ngx_conf_param(ngx_conf_t *cf);
char *ngx_conf_parse(ngx_conf_t *cf, ngx_str_t *filename);
char *ngx_conf_include(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


ngx_int_t ngx_conf_full_name(ngx_cycle_t *cycle, ngx_str_t *name,
    ngx_uint_t conf_prefix);
ngx_open_file_t *ngx_conf_open_file(ngx_cycle_t *cycle, ngx_str_t *name);
void ngx_cdecl ngx_conf_log_error(ngx_uint_t level, ngx_conf_t *cf,
    ngx_err_t err, const char *fmt, ...);


char *ngx_conf_set_flag_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_str_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_str_array_slot(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
char *ngx_conf_set_keyval_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_num_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_size_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_off_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_msec_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_sec_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_bufs_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_enum_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_bitmask_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


extern ngx_uint_t     ngx_max_module;
extern ngx_module_t  *ngx_modules[];


#endif /* _NGX_CONF_FILE_H_INCLUDED_ */
