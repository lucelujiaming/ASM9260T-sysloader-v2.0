/*
 * R/O VFS  - heyong@alpscale.com - ported from u-boot to NuwaStone
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#ifndef _VFS_H_
#define _VFS_H_

struct filesystem {
	int	(*registerfs)(block_dev_desc_t * cur_dev,int arg);
	int	(*detect)(block_dev_desc_t * cur_dev);
	int	(*ls)(block_dev_desc_t * cur_dev,const char *dir);
	long (*read)(block_dev_desc_t * cur_dev,const char *filename, void *buffer,unsigned long maxsize);
	char *name;
};

/* The current working directory */
#define CWD_LEN		511
// char file_cwd[CWD_LEN+1] = "/";

extern struct filesystem_info fs_info;

typedef struct filesystem_info{
    int  count;    // Number of Filesystems
    struct filesystem ** entry;  // fs entries

    int  current_dev; // current devices
    struct blk_dev_list *  blkdev_list; // device entries

    char file_cwd[CWD_LEN+1]; // current path
} fs_t ;

extern long __FS_START;
extern long __FS_END;

#define __add_filesystem(fs) \
	static struct filesystem * init_struct_##fs \
	__attribute__((__section__(".fs"))) = &fs

void fs_init(void);
struct filesystem * fs_get(char * name, int * index);
int print_pwd(void);
inline struct filesystem_info * get_fs_info(void);
const char * file_getfsname(int idx);
int file_cd(const char *path);
int file_detectfs(block_dev_desc_t * cur_dev);
int file_ls(block_dev_desc_t * cur_dev,const char *dir);
long file_read(block_dev_desc_t * cur_dev,const char *filename, void *buffer, unsigned long maxsize);

int do_mount(cmd_tbl_t * cmdtp,int argc, char *argv[]);
int do_umount(cmd_tbl_t * cmdtp,int argc, char *argv[]);
int do_ls(cmd_tbl_t * cmdtp,int argc, char *argv[]);
int do_cd(cmd_tbl_t * cmdtp,int argc, char *argv[]);
int do_pwd(cmd_tbl_t * cmdtp,int argc, char *argv[]);
int do_fread(cmd_tbl_t * cmdtp,int argc, char *argv[]);


#endif /* _VFS_H_ */
