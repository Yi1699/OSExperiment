#include "Ext2.h"

int main()
{
    login();
    int j = initialize_disk();
    if(!j) printf("The Ext2 file system initialization successfully completed!\n");
    initialize_memory();
    shell();
    return 0;
}