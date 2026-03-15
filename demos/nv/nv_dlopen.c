#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

// 定义函数指针类型
typedef int (*cfg_get_item_t)(char *key, char *value, int len);
typedef int (*cfg_set_t)(char *key, char *value);
typedef int (*cfg_unset_t)(char *key);
typedef void (*cfg_reset_t)(void);
typedef void (*cfg_save_t)(void);

#define CFGLEN 256
#define cfg_get(key, value) cfg_get_item(key, value, CFGLEN)

// 库句柄和函数指针（可以在程序启动时加载一次）
static void *nvram_handle = NULL;
static cfg_get_item_t cfg_get_item = NULL;
static cfg_set_t cfg_set_ptr = NULL;
static cfg_save_t cfg_save_ptr = NULL;

int load_nvram_lib(void)
{
    if (nvram_handle)
        return 0;

    nvram_handle = dlopen("/lib/libnvram.so", RTLD_LAZY);
    if (!nvram_handle)
    {
        fprintf(stderr, "Failed to load libnvram.so: %s\n", dlerror());
        return -1;
    }

    cfg_get_item = (cfg_get_item_t)dlsym(nvram_handle, "cfg_get_item");
    cfg_set_ptr = (cfg_set_t)dlsym(nvram_handle, "cfg_set");
    cfg_save_ptr = (cfg_save_t)dlsym(nvram_handle, "cfg_save");

    if (!cfg_get_item || !cfg_set_ptr || !cfg_save_ptr)
    {
        fprintf(stderr, "Failed to find symbols: %s\n", dlerror());
        dlclose(nvram_handle);
        nvram_handle = NULL;
        return -1;
    }

    return 0;
}

// 包装函数，保持代码风格一致
int cfg_set(char *key, char *value)
{
    if (load_nvram_lib() != 0)
        return -1;
    return cfg_set_ptr(key, value);
}

void cfg_save(void)
{
    if (load_nvram_lib() != 0)
        return;
    cfg_save_ptr();
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    char buf[CFGLEN];
    char imei[32] = {0};

    // 加载库
    if (load_nvram_lib() != 0)
    {
        return EXIT_FAILURE;
    }

    cfg_get_item("imei", buf, CFGLEN);
    strcpy(imei, buf);
    printf("IMEI=");
    printf(imei);
    printf("\n");

    char key[] = "test_anysoft";
    char value[] = "anysoft";

    if (cfg_set(key, value) == 0)
    {
        printf("Successfully set %s to %s.\n", key, value);
    }
    else
    {
        fprintf(stderr, "Failed to set %s.\n", key);
        return EXIT_FAILURE;
    }

    cfg_save();
    printf("Configuration saved successfully.\n");

    // 清理
    if (nvram_handle)
    {
        dlclose(nvram_handle);
    }

    return EXIT_SUCCESS;
}
