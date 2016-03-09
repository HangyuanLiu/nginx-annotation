
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CYCLE_H_INCLUDED_
#define _NGX_CYCLE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef NGX_CYCLE_POOL_SIZE
#define NGX_CYCLE_POOL_SIZE     NGX_DEFAULT_POOL_SIZE
#endif


#define NGX_DEBUG_POINTS_STOP   1
#define NGX_DEBUG_POINTS_ABORT  2


typedef struct ngx_shm_zone_s  ngx_shm_zone_t;

typedef ngx_int_t (*ngx_shm_zone_init_pt) (ngx_shm_zone_t *zone, void *data);
/* 共享内存相关信息
 * nginx封装数据结构 */
struct ngx_shm_zone_s {
    void                     *data;
    ngx_shm_t                 shm;//共享内存实际位置
    ngx_shm_zone_init_pt      init;/* 初始回调函数 */
    void                     *tag;//使用tag字段作为冲突标识
    ngx_uint_t                noreuse;  /* unsigned  noreuse:1; */
};


struct ngx_cycle_s {
	/*
	 * 保存这所有模块存储配置项的结构体的指针，它首先是一个数组，每个数组成员又是一个指针，
	 * 这个指针指向量一个存储指针的数组，因此会看到void****
	 * */
    void                  ****conf_ctx;
    //内存池
    ngx_pool_t               *pool;
    /*
     * 日志模块中提供了生成基本ngx_log_t日志对象的功能，这里的log实际上是在还没有执行
     * ngx_init_cycle方法钱，也就是还没有解析配置前，如果有信息需要输出到日志，就会暂时
     * 使用log对象，它会输出到屏幕上。在ngx_init_cycle方法执行后，将会根据nginx.conf配置文件
     * 中的配置向，构造出正确的日志文件，此时会对log重新赋值
     * */
    ngx_log_t                *log;
    /*
     * 由nginx.conf配置文件读取到日志文件路径后，将开始初始化error_log日志文件，由于log对象
     * 还在用于输出日志到屏幕，这时会用new_log对象暂时性地替代log日志，待初始化成功后，会用
     * new_log的地址覆盖上面的log指针
     * */
    ngx_log_t                 new_log;

    ngx_uint_t                log_use_stderr;  /* unsigned  log_use_stderr:1; */
    /*
     * 对于poll,rtsig这样的事件模块，。。。。。。。。。。
     * */
    ngx_connection_t        **files;
    //可用连接池，指向第一个ngx_connection_t空闲连接，一旦有用户发起连接时就从free_connections指向的链表头获取一个空闲连接
    ngx_connection_t         *free_connections;
    //表示当前可用连接数
    ngx_uint_t                free_connection_n;
    //双向链表容器，元素类型是ngx_connection_t结构体，表示可重复使用连接队列
    ngx_queue_t               reusable_connections_queue;
    /*每个数组元素都是ngx_listening_t,每个ngx_listening_t 代表一个Nginx服务监听端口及相关参数*/
    ngx_array_t               listening;
    /*
     * 动态数组容器，它保存着nginx所有要操作的目录。如果有目录不存在，则会试图创建，而创建目录失败
     * 将会导致nginx启动失败。
     * */
    ngx_array_t               paths;
    /*
     * 单链表容器，元素类型是ngx_open_file结构体，它表示nginx已经打开的所有文件。事实上，nginx框架
     * 不会向open_files链表中添加文件，而是由对此感兴趣的模块向其中添加文件路径名，nginx框架会在
     * ngx_init_cycle方法中打开这些文件
     * */
    ngx_list_t                open_files;
    /* Nginx使用的所有的共享内存都以list链表进行遍历查找以及冲突检测
     * 在创建新的共享内存之前会先对该链表进行遍历查找以及冲突检测.成功的直接返回引用.
     * 在配置文件全部解析完后,Nginx遍历该链表并逐个进行实际创建,即分配内存,管理机制初始化
     * */
    ngx_list_t                shared_memory;

    /* 当前进程中所有连接对象的总数,表示一个工作进程的最大可承受连接数
     * 与下面的connections成员配合使用
     * Nginx认为每一个连接一定至少需要一个读事件,一个写事件.而读事件,写事件,连接池是有3个大小相同的数组组成
     * 所以根据数组序号就可以一一对应
     * */
    ngx_uint_t                connection_n;
    ngx_uint_t                files_n;

    ngx_connection_t         *connections;//指向当前进程中的所有连接对象,与connection_n配合使用
    //可以理解我事件池，Nginx认为每一个连接一定至少需要一个读事件和一个写事件，所以分配三个大小相同的数组
    //分别表示读事件，写事件，连接池
    ngx_event_t              *read_events;//指向当前进程中的所有读事件对象,connection_n同时表示所有读事件的总数
    ngx_event_t              *write_events;//同上

    ngx_cycle_t              *old_cycle;
    //配置文件相对于安装目录的路径名称
    ngx_str_t                 conf_file;
    //nginx处理配置文件时需要特殊处理的在命令行携带的参数，一般是 -g选项携带的参数
    ngx_str_t                 conf_param;
    //nginx配置文件所在目录的路径
    ngx_str_t                 conf_prefix;
    //nginx安装目录的路径
    ngx_str_t                 prefix;
    //用于进程间同步的文件锁名称
    ngx_str_t                 lock_file;
    //使用gethostname系统调用得到的主机名
    ngx_str_t                 hostname;
};

//ngx_core_module的配置数据结构,本质上也是一个conf结构体，被cycle->conf_ctx指针所指向
typedef struct {
     ngx_flag_t               daemon;	//守护进程标志
     ngx_flag_t               master;

     ngx_msec_t               timer_resolution;

     ngx_int_t                worker_processes;
     ngx_int_t                debug_points;

     ngx_int_t                rlimit_nofile;
     off_t                    rlimit_core;

     int                      priority;

     ngx_uint_t               cpu_affinity_n;
     uint64_t                *cpu_affinity;

     char                    *username;
     ngx_uid_t                user;
     ngx_gid_t                group;

     ngx_str_t                working_directory;
     ngx_str_t                lock_file;

     ngx_str_t                pid;
     ngx_str_t                oldpid;

     ngx_array_t              env;
     char                   **environment;
} ngx_core_conf_t;


#define ngx_is_init_cycle(cycle)  (cycle->conf_ctx == NULL)


ngx_cycle_t *ngx_init_cycle(ngx_cycle_t *old_cycle);
ngx_int_t ngx_create_pidfile(ngx_str_t *name, ngx_log_t *log);
void ngx_delete_pidfile(ngx_cycle_t *cycle);
ngx_int_t ngx_signal_process(ngx_cycle_t *cycle, char *sig);
void ngx_reopen_files(ngx_cycle_t *cycle, ngx_uid_t user);
char **ngx_set_environment(ngx_cycle_t *cycle, ngx_uint_t *last);
ngx_pid_t ngx_exec_new_binary(ngx_cycle_t *cycle, char *const *argv);
uint64_t ngx_get_cpu_affinity(ngx_uint_t n);
ngx_shm_zone_t *ngx_shared_memory_add(ngx_conf_t *cf, ngx_str_t *name,
    size_t size, void *tag);


extern volatile ngx_cycle_t  *ngx_cycle;
extern ngx_array_t            ngx_old_cycles;
extern ngx_module_t           ngx_core_module;
extern ngx_uint_t             ngx_test_config;
extern ngx_uint_t             ngx_quiet_mode;


#endif /* _NGX_CYCLE_H_INCLUDED_ */
