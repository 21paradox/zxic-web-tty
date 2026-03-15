#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

// 定义函数指针类型
typedef int (*get_modem_info_t)(const char *command, const char *format, void *output);

int exec_at(int a2)
{
    char *v5 = NULL;
    char *s = NULL;
    const char *command = *(const char **)(a2 + 4);

    if (!strncmp(command, "at", 2) || !strncmp(command, "AT", 2))
    {
        v5 = (char *)malloc(0x400);
        s = (char *)malloc(0x40);

        if (!v5 || !s)
        {
            fprintf(stderr, "Memory allocation failed\n");
            free(v5);
            free(s);
            return -1;
        }

        sprintf(s, "%s\r\n", command);

        // 动态加载库
        void *handle = dlopen("/lib/libatutils.so", RTLD_LAZY);
        if (!handle)
        {
            fprintf(stderr, "Failed to load libatutils.so: %s\n", dlerror());
            free(v5);
            free(s);
            return -1;
        }

        get_modem_info_t get_modem_info = (get_modem_info_t)dlsym(handle, "get_modem_info");
        if (!get_modem_info)
        {
            fprintf(stderr, "Failed to find get_modem_info: %s\n", dlerror());
            dlclose(handle);
            free(v5);
            free(s);
            return -1;
        }

        if (get_modem_info(s, "%s", (void *)&v5) != 0)
        {
            fprintf(stderr, "get_modem_info failed\n");
        }
        else
        {
            printf("Response:%s\n", v5);
        }

        dlclose(handle);
        free(v5);
        free(s);
    }
    else
    {
        puts("command error");
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <AT Command>\n", argv[0]);
        return 1;
    }

    char *inputCommand = argv[1];
    char *args[2] = {NULL, inputCommand};
    exec_at((int)&args);
    return 0;
}
