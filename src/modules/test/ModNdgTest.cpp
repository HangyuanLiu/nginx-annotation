#include "NdgTestInit.hpp"
/*
 * 定义Nginx编译所需的ngx_module_t变量,这恰恰反应了Nginx模块的本质
 * ngx_module_t 就是模块
 * */
ngx_module_t ndg_test_module = NdgTestInit::module();
