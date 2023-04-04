/*

Alpha Scale AS3310X alpos
zhang bo, AlpScale Software Engineering,


------------------- Version 1.0  ----------------------
Create File
 zhangbo 2007-12-6

*/

#ifndef  __TASK_H__
#define  __TASK_H__


/* task configure */
#define MAX_TASK   5
#define TASK_HIGH  0
#define TASK_LOW   1

/* task number in high level*/
#define TASK_TIMER      0
#define TASK_COMMAND    1
/* task number in low level*/

#define task_count_base 1
#define NAME_LEN        32

typedef struct task_ops{
    const char *tname;
    int     tid;    //eg:1102 1:loaded. 1:low level. 02:number of task
    int     (*task_init)(void);
    int     (*task_out)(void);
    int     (*task_in)(void);
    int     (*tfunc)(void);
    int     (*task_release)(void);
    void*   save_data;
    //int  (*tioctl)(unsigned int cmd,unsigned long arg);
}t_ops;

int task_temp(void);
int task_loop(void);
int task_mark(int level,int number,int (*func)(void));
int task_unmark(int level,int number);
int high_task(void);
int low_task(void);
void main_task(void);
void do_init_task(void);

int ops_init(void);
int ops_tfunc(void);
int do_del(cmd_tbl_t *cmdtp,int argc,char * argv[]);
int do_add(cmd_tbl_t *cmdtp,int argc,char * argv[]);
int do_ps(cmd_tbl_t *cmdtp,int argc,char * argv[]);

extern int  task_count;
extern int  task_count_high;
extern int  task_count_low;
extern char high_task_set;
extern int  task_mask[2];
extern int (*task_chain[2][MAX_TASK])(void);
extern struct task_ops *task_op[2][MAX_TASK];

#endif
