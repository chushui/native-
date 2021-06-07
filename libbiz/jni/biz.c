#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <jni.h>
#include <android/log.h>
#include "xhook.h"
#include "malloc.h"

static size_t size = 0;

static int my_system_log_print(int prio, const char* tag, const char* fmt, ...)
{
    va_list ap;
    char buf[1024];
    int r;
    
    snprintf(buf, sizeof(buf), "[%s] %s", (NULL == tag ? "" : tag), (NULL == fmt ? "" : fmt));

    va_start(ap, fmt);
    r = __android_log_vprint(prio, "xhook_system", buf, ap);
    va_end(ap);
    return r;
}

static int my_libtest_log_print(int prio, const char* tag, const char* fmt, ...)
{
    va_list ap;
    char buf[1024];
    int r;
    
    snprintf(buf, sizeof(buf), "[%s] %s", (NULL == tag ? "" : tag), (NULL == fmt ? "" : fmt));

    va_start(ap, fmt);
    r = __android_log_vprint(prio, "xhook_libtest", buf, ap);
    va_end(ap);
    return r;
}

void *my_malloc(size_t size)
{
    void * p = malloc(size);
    size_t temp = malloc_usable_size(p);
    temp = size + temp;
    __android_log_print(ANDROID_LOG_DEBUG, "123", "my_malloc %zu\n", temp);
    return p;
}

void* my_calloc(size_t __item_count, size_t __item_size) {
    void * p = calloc(__item_count, __item_size);
    size_t temp = malloc_usable_size(p);
    size = size + temp;
    __android_log_print(ANDROID_LOG_DEBUG, "123", "my_calloc %zu\n", temp);
    return p;
}

void* my_realloc(void* __ptr, size_t __byte_count) {
    size_t temp = malloc_usable_size(__ptr);
    temp = size - temp;
    void * p = realloc(__ptr, __byte_count);
    temp = malloc_usable_size(p);
    size = size + temp;
    __android_log_print(ANDROID_LOG_DEBUG, "123", "my_realloc %zu\n", temp);
    return p;
}

void my_free(void* __ptr)
{
    size_t temp =  malloc_usable_size(__ptr);
    __android_log_print(ANDROID_LOG_DEBUG, "123", "my_free %zu\n", temp);
    temp = size + temp;
    free(__ptr);
}

void Java_com_qiyi_biz_NativeHandler_start(JNIEnv* env, jobject obj)
{
    (void)env;
    (void)obj;

    xhook_register("^/system/.*\\.so$",  "__android_log_print", my_system_log_print,  NULL);
    xhook_register("^/vendor/.*\\.so$",  "__android_log_print", my_system_log_print,  NULL);
    xhook_register(".*/libtest\\.so$", "__android_log_print", my_libtest_log_print, NULL);

    xhook_register(".*/libtest\\.so$", "malloc", my_malloc, NULL);
    xhook_register(".*/libtest\\.so$", "free", my_free, NULL);
    xhook_register(".*/libtest\\.so$", "realloc", my_realloc, NULL);
    xhook_register(".*/libtest\\.so$", "calloc", my_calloc, NULL);

    //just for testing
    xhook_ignore(".*/liblog\\.so$", "__android_log_print"); //ignore __android_log_print in liblog.so
    xhook_ignore(".*/libjavacore\\.so$", NULL); //ignore all hooks in libjavacore.so
}
