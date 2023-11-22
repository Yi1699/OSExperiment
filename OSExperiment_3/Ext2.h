#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>
#include <time.h>

#define NAME_LEN 16
#define BLOCK_SIZE 512

unsigned short fopen_table[16] ; /*文件打开表，最多可以同时打开16个文件*/
unsigned short last_alloc_inode; /*上次分配的索引结点号*/
unsigned short last_alloc_block; /*上次分配的数据块号*/
unsigned short current_dir;  /*当前目录(索引结点）*/
char current_path[256];   	 /*当前路径(字符串) */
struct ext2_group_desc group_desc;	 /*组描述符*/ 
FILE * fil;
char* Path = "Disk";

/*初始化文件系统*/
int initialize_disk();	 /*建立文件系统*/
void initialize_memory();	 /*初始化文件系统的内存数据*/

/*底层函数*/
void update_group_desc(); /*将内存中的组描述符更新到"硬盘".*/
void reload_group_desc(); /*载入可能已更新的组描述符*/	
void load_inode_entry();  /*载入特定的索引结点*/
void update_inode_entry(); /*更新特定的索引结点*/
void load_block_entry(); /*载入特定的数据块*/
void update_block_entry();	/*更新特定的数据块*/
void update_inode_i_block(); /*根据多级索引机制更新索引结点的数据块信息域*/
void ext2_new_inode(); /*分配一个新的索引结点*/
void ext2_alloc_block(); /*分配一个新的数据块*/
void ext2_free_inode(__u16 inode); /*释放特定的索引结点*/
void ext2_free_block_bitmap(__u16 block_num); /*释放特定块号的数据块位图*/
void ext2_free_blocks();	/*释放特定文件的所有数据块*/
void search_filename();	/*在当前目录中查找文件*/
int test_fd(int number);          /*检测文件打开ID(fd)是否有效*/
int find_block_addr(unsigned short node); /*给定索引结点号，返回索引结点在磁盘的偏移量*/

/*用户调用接口*/
void dir();			/*无参数*/	
void mkdir();			/*filename*/	
void rmdir();		/*filename*/	
int create(const char* name, __u16 mode);			/*filename*/	
void delete_();			/*filename*/	
void cd();			/*filename*/
void attrib();			/*filename，rw*/
void open();			/*filename*/	
void close();			/*fd*/
void read();			/*fd*/
void write();			/*fd，source*/
void shell();		/*启动用户接口*/
void format();		/*重新建立文件系统,无参数*/
void quit();		/*退出shell(),无参数*/
void ls();

/*组描述符，占32字节，需填充块内剩余480字节*/
struct ext2_group_desc 
{
    char bg_volume_name[16];    //卷名
    __u16 bg_block_bitmap;      //保存块位图的块号
    __u16 bg_inode_bitmap;      //保存索引结点位图的块号
    __u16 bg_inode_table;       //索引结点表的起始块号
    __u16 bg_free_blocks_count; //本组空闲块的个数
    __u16 bg_free_inodes_count; //本组空闲索引结点的个数
    __u16 bg_used_dirs_count;   //本组目录的个数
    char bg_pad[4];             //填充(0xff)
};

/*索引结点，占64字节*/
struct ext2_inode
{
    __u16 i_mode;       //文件类型及访问权限
    __u16 i_blocks;     //文件的数据块个数
    __u32 i_size;       //大小(字节)
    __u64 i_atime;      //访问时间
    __u64 i_ctime;      //创建时间
    __u64 i_mtime;      //修改时间
    __u64 i_dtime;      //删除时间
    __u16 i_block[8]    //指向数据块的指针
    char i_pad[8]       //填充1(0xff)
}

/*文件和目录*/
struct ext2_dir_entry
{ 
    __u16 inode;                            //索引节点号
    __u16 rec_len;                          //目录项长度
    __u8 name_len;                          //文件名长度
    __u8 file_type;                         //文件类型(1:普通文件，2:目录…)
    char name[NAME_LEN];  //文件名
}; 

int initialize_disk()	 /*建立文件系统*/
{
    fil = fopen(Path, "r");
    if(!fil)
    {
        fil = fopen(Path, "wb+");
    }
    else fseek(fil, 0, SEEK_SET);
    __u32 init_array[128]{0};
    fwrite(init_array, sizeof(init_array), 4611, fil);
    //写入组描述符
    strcpy(group_desc.bg_volume_name, "Root");
    group_desc.bg_block_bitmap = 1;
    group_desc.bg_inode_bitmap = 2;
    group_desc.bg_inode_table = 3;
    group_desc.bg_free_blocks_count = 4095;    //除去一个根目录
    group_desc.bg_free_inodes_count = 4095;
    group_desc.bg_used_dirs_count = 1;//根目录
    strcpy(group_desc.bg_pad, "0000");
    fseek(fil, 0, SEEK_SET);
    fwrite(group_desc, sizeof(ext2_group_desc), 1, fil);
    time_t curr;
    time(&curr);
    //根目录索引结点
    struct ext2_inode root;//0000_0010_0000_0111;
    root.i_mode = 0x0207;
    root.i_blocks = 1;
    root.i_size = 512;
    root.i_atime = curr;
    root.i_ctime = curr;
    root.i_mtime = curr;
    root.i_dtime = 0;
    root.i_block[0] = 0;//第一块数据块
    fseek(fil, group_desc.bg_inode_table * BLOCK_SIZE, SEEK_SET);
    fwrite(root, sizeof(ext2_inode), 1, fil);

    struct ext2_dir_entry dir_curr;//当前目录
    dir_curr.inode = 1;//索引结点从1开始计数，数据块从0开始计数
    dir_curr.rec_len = 64;
    dir_curr.name_len = NAME_LEN;
    strcpy(dir_curr.name, ".");
    fseek(fil, 515 * BLOCK_SIZE, SEEK_SET);
    fwrite(dir_curr, sizeof(dir_curr), 1, fil);

    struct ext2_dir_entry dir_back;//上级目录
    dir_back.inode = 0;
    dir_back.rec_len = 64;
    dir_back.name_len = NAME_LEN;
    strcpy(dir_back.name, "..");
    fwrite(dir_curr, sizeof(dir_back.name), 1, fil);

    fseek(fil, group_desc.bg_block_bitmap * BLOCK_SIZE, SEEK_SET);
    __u16 bitchange = 0x8000;//修改位图第一位为1
    fwrite(bitchange, sizeof(bitchange), 1, fil);
    fseek(fil, group_desc.bg_inode_bitmap, SEEK_SET);
    fwrite(bitchange, sizeof(bitchange), 1, fil);

    fclose(fil);
    return 0;
}

void initialize_memory()
{
    last_alloc_inode = 1; /*上次分配的索引结点号*/
    last_alloc_block = 0; /*上次分配的数据块号*/
    current_dir = 1;  /*当前目录(索引结点）*/
    for(int i = 0;i < 16; i++) fopen_table[i] = 0;
    strcpy(current_path, "Root/");//当前路径
}

void update_group_desc()
{
    FILE *fil = fopen(Path, "wb+");
    fseek(fil, 0, SEEK_SET);
    fwrite(group_desc, sizeof(group_desc), 1, fil);
    fclose(fil);
}

int test_fd(int number){
    if(fopen_table[number] == 0){                  // 0不会分配给文件
        return 0;
    }
    else if(fopen_table[number] > 0)return 1;
    else return 2;
}
void load_inode_entry(__u16 mode, __u16 blocks, __u16 size)
{

}

int dir_entry_position(int dir_entry_begin, short i_block[8]) // 目录体的相对开始字节
{
    int dir_blocks = dir_entry_begin / 512;   // 存储目录需要的块数
    int block_offset = dir_entry_begin % 512; // 块内偏移字节数
    int a;
    FILE *fp = NULL;
    if (dir_blocks <= 5) //前六个直接索引
        return 515 * BLOCK_SIZE + i_block[dir_blocks] * BLOCK_SIZE + block_offset;
    else //间接索引
    {
        while (fp == NULL)
        fp = fopen(Path, "wb+");
        dir_blocks = dir_blocks - 6;
        if (dir_blocks < 128) //一个块 512 字节，一个int为 4 个字节 一级索引有 512/4= 128 个
            {
                int a;
                fseek(fp, 515 * BLOCK_SIZE + i_block[6] * BLOCK_SIZE + dir_blocks * 4, SEEK_SET);
                fread(&a, sizeof(int), 1, fp);
                return 515 * BLOCK_SIZE + a * BLOCK_SIZE + block_offset;
            }
        else //二级索引
        {
            dir_blocks = dir_blocks - 128;
            fseek(fp, 515 * BLOCK_SIZE + i_block[7] * BLOCK_SIZE + dir_blocks / 128 * 4, SEEK_SET);
            fread(&a, sizeof(int), 1, fp);
            fseek(fp, 515 * BLOCK_SIZE + a * BLOCK_SIZE + dir_blocks % 128 * 4, SEEK_SET);
            fread(&a, sizeof(int), 1, fp);
            return 515 * BLOCK_SIZE + a * BLOCK_SIZE + block_offset;
        }
        fclose(fp);
    }
}


int create(const char* name, __u16 mode)
{
    __u16 modex = mode >> 8;
    if(modex < 0 || modex > 7)
    {
        printf("Type Error!");
        return -1;
    }
    char Name[NAME_LEN];
    int name_len = strlen(name);
    if(name_len >= NAME_LEN)
    {
        name_len = NAME_LEN;
        strncpy(Name, name, NAME_LEN);
    }
    else strcpy(Name, name);
    FILE *fil = fopen(Path, "wb+");
    fseek(fil,find_block_addr(current_dir) , SEEK_SET);
    fread()
}

int find_block_addr(unsigned short node)
{
    return (group_desc.inode_table + ((current_dir - 1) * sizeof(ext2_inode) / BLOCK_SIZE)) * BLOCK_SIZE + ((current_dir - 1) * sizeof(ext2_inode) % BLOCK_SIZE);
}