#include <stdio.h>
#include <stdlib.h>

#define PROCESS_NAME_LEN 32     /*进程名长度*/
#define MIN_SLICE 10            /*最小碎片的大小*/
#define DEFAULT_MEM_SIZE 1024   /*内存大小*/
#define DEFAULT_MEM_START 0     /*起始位置*/
/* 内存分配算法 */
#define MA_FF 1
#define MA_BF 2
#define MA_WF 3

/*描述每一个空闲块的数据结构*/
struct free_block_type{
    int size;
    int start_addr;
    struct free_block_type *next;
};

/*每个进程分配到的内存块的描述*/
struct allocated_block{
    int pid;
    int size;
    int start_addr;
    char process_name[PROCESS_NAME_LEN];
    struct allocated_block *next;
};

int mem_size=DEFAULT_MEM_SIZE;  /*内存大小*/
int ma_algorithm = MA_FF;       /*当前分配算法*/
static int pid = 0;             /*初始pid*/
int flag = 0;                   /*内存设置标志位*/
int num_free = 0;
/*指向内存中空闲块链表的首指针*/
struct free_block_type *free_block = NULL;

/*进程分配内存块链表的首指针*/
struct allocated_block *allocated_block_head = NULL;

/*设置内存大小标志*/
/*初始化空闲块，默认为一块，可以指定大小及起始地址*/
struct free_block_type* init_free_block(int mem_size){
    struct free_block_type *fb;

    fb=(struct free_block_type *)malloc(sizeof(struct free_block_type));
    if(fb==NULL){
        printf("No mem\n");
        return NULL;
    }
    fb->size = mem_size;
    fb->start_addr = DEFAULT_MEM_START;
    fb->next = NULL;
    num_free = 1;
    return fb;
};

void rearrange(int algorithm);
void rearrange_FF();
void rearrange_BF();
void rearrange_WF();
int free_mem(struct allocated_block *ab);
int dispose(struct allocated_block *free_ab);
int allocate_mem(struct allocated_block *ab);

/*显示菜单*/
void display_menu(){
    printf("\n");
    printf("1 - Set memory size (default=%d)\n", DEFAULT_MEM_SIZE);
    printf("2 - Select memory allocation algorithm\n");
    printf("3 - New process \n");
    printf("4 - Terminate a process \n");
    printf("5 - Display memory usage \n");
    printf("0 - Exit\n");
}

/*设置内存的大小*/
int set_mem_size(){
    int size;
    if(flag!=0){  //防止重复设置
        printf("Cannot set memory size again\n");
        return 0;
    }
    printf("Total memory size =");
    scanf("%d", &size);
    if(size>0) {
        mem_size = size;
        free_block->size = mem_size;
    }
    flag=1;
    return 1;
}

/* 设置当前的分配算法 */
void set_algorithm(){
    int algorithm;
    printf("\t1 - First Fit\n");
    printf("\t2 - Best Fit\n");
    printf("\t3 - Worst Fit\n");
    scanf("%d", &algorithm);
    if(algorithm>=1 && algorithm <=3)
    {
        ma_algorithm=algorithm;
        //按指定算法重新排列空闲区链表
        rearrange(ma_algorithm); 
    }
}

/*按指定的算法整理内存空闲块链表*/
void rearrange(int algorithm){
    switch(algorithm){
        case MA_FF:  rearrange_FF(); break;
        case MA_BF:  rearrange_BF(); break;
        case MA_WF: rearrange_WF(); break;
    }
}

//首次适应算法
void rearrange_FF()
{//采用选择排序方法将空闲链表按地址从小到大排序
    //min_addr->next为最小结点
    struct free_block_type* min_addr = free_block;
    //frt用于标志已完成排序的结点，pref为frt前序结点，pre为mov前序结点，mov用于遍历找地址最小结点
    struct free_block_type* frt, *pref, *pre, *mov;
    if(!free_block) return;
    else if(!free_block->next) return;
    pref = free_block; frt = free_block;pre = free_block;mov = free_block->next;
    while(frt->next)
    {
        min_addr = frt;
        while(mov && min_addr->next)
        {
            if(mov->start_addr < min_addr->next->start_addr)
            {
                min_addr = pre;
                mov = mov->next;
                pre = pre->next;
            }
            else
            {
                pre = pre->next;
                mov = mov->next;
            }
        }
        if(frt->start_addr < min_addr->next->start_addr)
        {
            if(frt != free_block) pref = frt;
            frt = frt->next;
            pre = frt;
            if(frt->next) mov = frt->next;
            continue;
        }
        mov = min_addr->next;
        min_addr->next = mov->next;
        mov->next = frt;
        if(frt == free_block)
        {
            free_block = mov;
            pref = free_block;
        }
        
        else pref->next = mov;
        mov = frt->next;
        pre = frt;
    }
}

//最优适应算法
void rearrange_BF()
{
    struct free_block_type* min_size = free_block;
    struct free_block_type *frt, *pref, *pre, *mov;
    if(!free_block) return;
    else if(!free_block->next) return;
    pref = free_block; frt = free_block;pre = free_block;mov = free_block->next;
    while(frt->next)
    {
        min_size = frt;
        while(mov&& min_size->next)
        {
            if(mov->size < min_size->next->size)
            {
                min_size = pre;
                mov = mov->next;
                pre = pre->next;
            }
            else
            {
                pre = pre->next;
                mov = mov->next;
            }
        }
        if(frt->size < min_size->next->size)
        {
            if(frt != free_block) pref = frt;
            frt = frt->next;
            pre = frt;
            if(frt->next) mov = frt->next;
            continue;
        }
        mov = min_size->next;
        min_size->next = mov->next;
        mov->next = frt;
        if(frt == free_block)
        {
            free_block = mov;
            pref = free_block;
        }
        else pref->next = mov;
        mov = frt->next;
        pre = frt;
    }
}

//最差适应算法
void rearrange_WF()
{
    struct free_block_type* max_size = free_block;
    struct free_block_type *frt, *pref, *pre, *mov;
    if(!free_block) return;
    else if(!free_block->next) return;
    pref = free_block; frt = free_block;pre = free_block;mov = free_block->next;
    while(frt->next)
    {
        max_size = frt;
        while(mov && max_size->next)
        {
            if(mov->size > max_size->next->size)
            {
                max_size = pre;
                mov = mov->next;
                pre = pre->next;
            }
            else
            {
                pre = pre->next;
                mov = mov->next;
            }
        }
        if(frt->size > max_size->next->size)
        {
            if(frt != free_block) pref = frt;
            frt = frt->next;
            pre = frt;
            if(frt->next) mov = frt->next;
            continue;
        }
        mov = max_size->next;
        max_size->next = mov->next;
        mov->next = frt;
        if(frt == free_block)
        {
            free_block = mov;
            pref = free_block;
        }
        else pref->next = mov;
        mov = frt->next;
        pre = frt;
    }
}


/*创建新的进程，主要是获取内存的申请数量*/
int new_process(){
    struct allocated_block *ab;
    int size;
    int ret;
    ab=(struct allocated_block *)malloc(sizeof(struct allocated_block));
    if(!ab) exit(-5);
    ab->next = NULL;
    pid++;
    sprintf(ab->process_name, "PROCESS-%02d", pid);
    ab->pid = pid;    
    printf("Memory for %s:", ab->process_name);
    scanf("%d", &size);
    if(size>0) ab->size = size;
    ret = allocate_mem(ab);  /* 从空闲区分配内存，ret==1表示分配ok*/
    /*如果此时allocated_block_head尚未赋值，则赋值*/
    if((ret==1) &&(allocated_block_head == NULL)){ 
        allocated_block_head=ab;
        return 1;
    }
    /*分配成功，将该已分配块的描述插入已分配链表*/
    else if (ret == 1) {
        ab->next = allocated_block_head;
        allocated_block_head = ab;
        return 2;
    }
    else if(ret == -1){ /*分配不成功*/
        printf("Allocation fail\n");
        free(ab);
        return -1;       
     }
    return 3;
}

/*分配内存模块*/
int allocate_mem(struct allocated_block *ab){
    struct free_block_type *fbt, *pre;
    int request_size=ab->size;
    fbt = free_block;
    pre = free_block;
    int isAllocated = -1;
    while(fbt)
    {
        if(fbt->size >= request_size && fbt->size - request_size <= MIN_SLICE)
        {//当找到的空闲区满足条件且剩余空间较小，则全部将其分配
            ab->start_addr = fbt->start_addr;
            ab->size = fbt->size;
            if(fbt == free_block) free_block = fbt->next;
            else
            {
                pre->next = fbt->next;
                if(fbt) free(fbt);
                num_free--;
            }
            isAllocated = 1;
            break;
        }
        else if(fbt->size >= request_size && fbt->size - request_size > MIN_SLICE)
        {//找到的空闲区满足条件而剩余空间较大，则分割后进行分配
            ab->start_addr = fbt->start_addr;
            fbt->size = fbt->size - request_size;
            fbt->start_addr = fbt->start_addr + request_size;
            isAllocated = 1;
            break;
        }
        if(fbt == free_block) fbt = fbt->next;
        else
        {
            fbt = fbt->next;
            pre = pre->next;
        }
    }
    if(isAllocated == -1)
    {   //若未找到合适的空闲区，判断剩余空间总和是否可用，可用则进行紧缩
        int free_sum = 0;
        fbt = free_block;
        while(fbt)
        {
            free_sum += fbt->size;
            fbt=fbt->next;
        }
        if(free_sum >= request_size)
        {//剩余空间总和满足需求，进行紧缩
            int start_temp = 0;
            struct allocated_block* p = allocated_block_head;
            int alloc_sum = 0;
            while(p)
            {//将分配好的进程全部移至内存前面
                p->start_addr = start_temp;
                start_temp = p->start_addr + p->size;
                alloc_sum += p->size;
                p = p->next;
            }
            fbt = free_block->next;
            pre = free_block;
            while(pre) 
            {//删除整个空闲分区链
                if(pre) free(pre);
                pre = fbt;
                if(fbt) fbt = fbt->next;//避免fbt为空
            }
            //剩余空间合并成一个大空闲分区
            free_block = (struct free_block_type*)malloc(sizeof(struct free_block_type));
            free_block->next = NULL;
            free_block->size = mem_size - start_temp;
            free_block->start_addr = start_temp;
            num_free = 1;
            isAllocated = allocate_mem(ab);
        }
        if(num_free == 0) free_block = NULL;
    }

    if(isAllocated == 1)
    {//分配好后按照相应算法排序
        rearrange(ma_algorithm);
    }
    return isAllocated;
}

struct allocated_block* find_process(int pid)
{
    struct allocated_block * s = allocated_block_head;
    while(s)
    {
        if(s->pid == pid) return s;
        else s = s->next;
    }
    return NULL;
}

/*删除进程，归还分配的存储空间，并删除描述该进程内存分配的节点*/
void kill_process(){
    struct allocated_block *ab;
    int pid;
    printf("Kill Process, pid=");
    scanf("%d", &pid);
    ab=find_process(pid);
    if(ab!=NULL){
        free_mem(ab); /*释放ab所表示的分配区*/
        dispose(ab);  /*释放ab数据结构节点*/
    }
}

/*将ab所表示的已分配区归还，并进行可能的合并*/
int free_mem(struct allocated_block *ab){
    struct free_block_type *fbt, *pre;
    fbt = (struct free_block_type*)malloc(sizeof(struct free_block_type));
    if(!fbt) return -1;
    //新结点插入到表头
    fbt->next = free_block;
    free_block = fbt;
    fbt->size = ab->size;
    fbt->start_addr = ab->start_addr;
    num_free++;
    rearrange_FF();//按地址大小有序排列
    pre = free_block;
    while(pre)
    {//遍历一遍链表，进行可能的合并
        if(pre->next != fbt && fbt != free_block)
        {
            pre = pre->next;
            continue;
        }
        if(fbt->next)
        {
            if(pre->start_addr + pre->size == fbt->start_addr && fbt->start_addr + fbt->size == fbt->next->start_addr)
            {//前后均与新加入结点相邻，则将三个分区合并
                pre->size = pre->size + fbt->size + fbt->next->size;
                pre->next = fbt->next->next;
                free(fbt->next);
                free(fbt);
                num_free -= 2;
                break;
            }
            else if(fbt->next->start_addr == fbt->size + fbt->start_addr)
            {//只有新节点后面的分区相邻，进行合并
                fbt->size = fbt->size + fbt->next->size;
                struct free_block_type *temp = fbt->next;
                fbt->next = fbt->next->next;
                free(temp);
                num_free--;
                break;
            }
        }
        if(pre->start_addr + pre->size == fbt->start_addr)
        {//只有前面的分区与新节点相邻，进行合并
            if(fbt->next) {if(fbt->start_addr + fbt->size == fbt->next->start_addr) break;}
            pre->size = pre->size + fbt->size;
            pre->next = fbt->next;
            free(fbt);
            num_free--;
            break;
        }
        break;
    }
    rearrange(ma_algorithm);//按相应算法排列
    return 1;
}

/*释放ab数据结构节点*/
int dispose(struct allocated_block *free_ab){
    struct allocated_block *pre, *ab;
    if(free_ab == allocated_block_head) { /*如果要释放第一个节点*/
        allocated_block_head = allocated_block_head->next;
        free(free_ab);
        return 1;
    }
    pre = allocated_block_head;  
    ab = allocated_block_head->next;
    while(ab!=free_ab){
        pre = ab;
        ab = ab->next;
    }
    pre->next = ab->next;
    free(ab);
    return 2;
}

/* 显示当前内存的使用情况，包括空闲区的情况和已经分配的情况 */
int display_mem_usage(){
    struct free_block_type *fbt=free_block;
    struct allocated_block *ab=allocated_block_head;

    printf("----------------------------------------------------------\n");

    /* 显示已分配区 */
    printf("Used Memory:\n");
    printf("%10s %20s %10s %10s\n", "PID", "ProcessName", "start_addr", " size");
    while(ab!=NULL){
        printf("%10d %20s %10d %10d\n", ab->pid, ab->process_name, ab->start_addr, ab->size);
        ab=ab->next;
    }
    if(fbt==NULL){
        printf("\nNo more memory can be used!\n");
        printf("----------------------------------------------------------\n");
        return(-1);
    } 
    /* 显示空闲区 */
    printf("\nFree Memory:\n");
    printf("%20s %20s\n", "      start_addr", "       size");
    while(fbt!=NULL){
        printf("%20d %20d\n", fbt->start_addr, fbt->size);
        fbt=fbt->next;
    }    

    printf("----------------------------------------------------------\n");
    return 0;
}

void do_exit()
{
    struct allocated_block* s = allocated_block_head;
    struct allocated_block* p = s; 
    while(s)
    {
        if(p) s = p->next;
        free(p);
        if(s) p = s;
    }
    struct free_block_type* sf = free_block;
    struct free_block_type* pf = sf; 
    while(sf)
    {
        if(pf) sf = pf->next;
        free(pf);
        if(sf) pf = sf;
    }
}

int main(){
    char choice;
    pid=0;
    free_block = init_free_block(mem_size); //初始化空闲区
    while(1) {
        display_menu();	//显示菜单
        fflush(stdin);
        choice=getchar();	//获取用户输入
        switch(choice){
            case '1': set_mem_size(); break; 	//设置内存大小
            case '2': set_algorithm(); flag=1; break;//设置算法
            case '3': new_process(); flag=1; break;//创建新进程
            case '4': kill_process(); flag=1;   break;//删除进程
            case '5': display_mem_usage(); flag=1; break;	//显示内存使用
            case '0': do_exit(); exit(0);	//释放链表并退出
            default: break;
        }
    } 
}
