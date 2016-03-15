#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct
{
	ngx_http_upstream_conf_t upstream;
} ngx_http_mytest_conf_t;
typedef struct
{
	ngx_http_status_t status;
}ngx_http_mytest_ctx_t;



static char* ngx_http_mytest(ngx_conf_t *cf,ngx_command_t *cmd,void *conf);


static ngx_command_t ngx_http_mytest_commands[] = {
		{
				ngx_string("mytest"),
				NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LMT_CONF|NGX_CONF_NOARGS,
				ngx_http_mytest,
				NGX_HTTP_LOC_CONF_OFFSET,
				0,
				NULL
		},
		ngx_null_command
};
static ngx_http_module_t ngx_http_mytest_module_ctx = {
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
};

ngx_module_t ngx_http_mytest_module = {
		NGX_MODULE_V1,
		&ngx_http_mytest_module_ctx,
		ngx_http_mytest_commands,
		NGX_HTTP_MODULE,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NGX_MODULE_V1_PADDING
};
static ngx_int_t ngx_http_mytest_handler(ngx_http_request_t *r)
{
	if(!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD)))
	{
		return NGX_HTTP_NOT_ALLOWED;
	}
	ngx_int_t rc = ngx_http_discard_request_body(r);
	if(rc != NGX_OK){
		return rc;
	}
	ngx_str_t type = ngx_string("text/plain");
	ngx_str_t response = ngx_string("Hello World");

	r->headers_out.status = NGX_HTTP_OK;
	r->headers_out.content_length_n = response.len;
	r->headers_out.content_type = type;

	rc = ngx_http_send_header(r);
	if(rc == NGX_ERROR || rc > NGX_OK || r->header_only){
		return rc;
	}

	/*
	ngx_buf_t *b;
	b = ngx_create_temp_buf(r->pool,response.len);
	if(b == NULL){
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}
	ngx_memcpy(b->pos,response.data,response.len);
	b->last = b->pos + response.len;
	b->last_buf = 1;

	ngx_chain_t out;
	out.buf = b;
	out.next = NULL;

	return ngx_http_output_filter(r,&out);
	*/

	ngx_buf_t *b;
	b = ngx_palloc(r->pool,sizeof(ngx_buf_t));
	u_char* filename = (u_char*)"tmp/test.txt";
	b->in_file = 1;
	b->file = ngx_pcalloc(r->pool,sizeof(ngx_file_t));
	b->file->fd = ngx_open_file(filename,NGX_FILE_RDONLY|NGX_FILE_NONBLOCK,NGX_FILE_OPEN,0);
	b->file->log = r->connection->log;
	b->file->name.data = filename;
	b->file->name.len = sizeof(filename) - 1;
	if(b->file->fd <= 0)
	{
		return NGX_HTTP_NOT_FOUND;
	}
	if(ngx_file_info(filename,&b->file->info) == NGX_FILE_ERROR)
	{
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}
	r->headers_out.content_length_n = b->file->info.st_size;
	b->file_pos = 0;
	b->file_last = b->file->info.st_size;
	b->last_buf = 1;

	ngx_chain_t out;
	out.buf = b;
	out.next = NULL;
	return ngx_http_output_filter(r,&out);
}

static char* ngx_http_mytest(ngx_conf_t *cf,ngx_command_t *cmd,void *conf)
{
	ngx_http_core_loc_conf_t *clcf;
	/*
	 * 首先找到mytest配置项所属的配置块，clcf看上去像是location块内的数据结构，
	 * 其实它可以是main,srv或者loc级别配置项,在每个http{}和server{}内也都有一个
	 * ngx_http_core_loc_conf_t结构体
	 * */
	clcf = ngx_http_conf_get_module_loc_conf(cf,ngx_http_core_module);
	/*
	 * http框架在处理用户请求进行到NGX_HTTP_CONTENT_PHASE阶段时，就将调用我们
	 * 实现的ngx_http_mytest_handler方法处理这个请求
	 * */
	clcf->handler = ngx_http_mytest_handler;
	return NGX_CONF_OK;
}
/*
static void* ngx_http_mytest_create_loc_conf(ngx_conf_t *cf)
{
	ngx_http_mytest_conf_t *mycf;
	mycf = (ngx_http_mytest_conf_t *)ngx_pcalloc(cf->pool,sizeof(ngx_http_mytest_conf_t));
	if(mycf == NULL){
		return NULL;
	}
	mycf->upstream.connect_timeout = 60000;
	mycf->upstream.send_timeout = 60000;
	mycf->upstream.read_timeout = 6000;
	mycf->upstream.store_access = 0600;

	mycf->upstream.buffering = 0;
	mycf->upstream.bufs.num = 8;
	mycf->upstream.bufs.size = ngx_pagesize;
	mycf->upstream.buffer_size = ngx_pagesize;
	mycf->upstream.busy_buffers_size = 2*ngx_pagesize;
	mycf->upstream.temp_file_write_size = 2*ngx_pagesize;
	mycf->upstream.max_temp_file_size = 1024*1024*1024;

	mycf->upstream.hide_headers = NGX_CONF_UNSET_PTR;
	mycf->upstream.pass_headers = NGX_CONF_UNSET_PTR;

	return mycf;
}

static char* ngx_http_mytest_merge_loc_conf(ngx_conf_t *cf,void *parent,void *child)
{
	ngx_http_mytest_conf_t *prev = (ngx_http_mytest_conf_t *)parent;
	ngx_http_mytest_conf_t *conf = (ngx_http_mytest_conf_t *)child;

	ngx_hash_init_t hash;
	hash.max_size = 100;
	hash.bucket_size = 1024;
	hash.name = "proxy_headers_hash";
	if(ngx_http_upstream_hide_headers_hash(cf,&conf->upstream,&prev->upstream,ngx_http_proxy_hide_headers,&hash)!=NGX_OK)
	{
		return 	NGX_CONF_ERROR;
	}
	return NGX_CONF_OK;
}

static ngx_int_t
mytest_upstream_create_request(ngx_http_request_t *r)
{
	static ngx_str_t backendQueryLine =
			ngx_string("GET /search?q=%V HTTP/1.1\r\nHost: www.baidu.com\r\nConnection: close\r\n\r\n");
	ngx_int_t queryLineLen = backendQueryLine.len + r->args.len -2;

	ngx_buf_t *b = ngx_create_temp_buf(r->pool,queryLineLen);
	if(b == NULL) return NGX_ERROR;
	b->last = b->pos + queryLineLen;

	ngx_snprintf(b->pos,queryLineLen,(char*)backendQueryLine.data,&r->args);
	r->upstream->request_bufs = ngx_alloc_chain_link(r->pool);
	if(r->upstream->request_bufs == NULL)
		return NGX_ERROR;
	r->upstream->request_bufs->buf = b;
	r->upstream->request_bufs->next = NULL;

	r->upstream->request_sent = 0;
	r->upstream->header_sent = 0;
	r->header_hash = 1;
	return NGX_OK;
}
*/
