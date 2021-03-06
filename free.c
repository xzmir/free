#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/vmmeter.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct uvmexp uvm;
int64_t phy_mem;
int64_t used_mem;
int64_t free_mem;
int64_t swap;
int64_t used_swap;
int64_t free_swap;

void output(int64_t size)
{
    if (size < 1024 * 10)
    {
        printf("%17lldB", size);
    }
    else if (size < 1024 * 1024 * 10)
    {
        printf("%17lldK", size / 1024);
    }
    else if (size < (int64_t)1024 * 1024 * 1024 * 10)
    {
        printf("%17lldM", size / (1024 * 1024));
    }
    else
    {
        printf("%17lldG", size / (1024 * 1024 * 1024));
    }
    return;
}

void usage(void)
{
    fprintf(stderr, "usage: free \n");
    exit(1);
}

int main(int argc, char **argv)
{
    int mib[2];
    size_t len;

    if (pledge("stdio ps vminfo", NULL) == -1)
    {
        err(1, "pledge");
    }

    if (argc > 1)
    {
        usage();
    }

    mib[0] = CTL_HW;
    mib[1] = HW_PHYSMEM64;
    len = sizeof(phy_mem);
    if (sysctl(mib, 2, &phy_mem, &len, NULL, 0) == -1)
    {
        err(1, "sysctl");
    }

    mib[0] = CTL_VM;
    mib[1] = VM_UVMEXP;
    len = sizeof(uvm);

    if (pledge("stdio vminfo", NULL) == -1)
    {
        err(1, "pledge");
    }

    if (sysctl(mib, 2, &uvm, &len, NULL, 0) == -1)
    {
        err(1, "sysctl");
    }

    if (pledge("stdio", NULL) == -1)
    {
        err(1, "pledge");
    }

    free_mem = (int64_t)uvm.pagesize * uvm.free;
    used_mem = phy_mem - free_mem;

    swap = (int64_t)uvm.pagesize * uvm.swpages;
    used_swap = (int64_t)uvm.pagesize * uvm.swpginuse;
    free_swap = swap - used_swap;

    printf("        %18s%18s%18s\n", "total", "used", "free");

    printf("Mem:    ");
    output(phy_mem);
    output(used_mem);
    output(free_mem);
    printf("\n");
    
    printf("Swap:   ");
    output(swap);
    output(used_swap);
    output(free_swap);
    printf("\n");

    return 0;
}

