/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

AS3310 Boot Loader Command Header file
 
Change log: 
------------------- Version 1.0  ----------------------
 Create File, define cmd struct,and sections
 He Yong 2006-11-06
*/

/*
 *  Definitions for Command Processor
 */
#ifndef __CMD_H
#define __CMD_H

#ifndef NULL
#define NULL	0
#endif

#ifndef	__ASSEMBLY__
/*
 * Monitor Command Table
 */

typedef struct cmd_tbl_s {
	char		*name;		/* Command Name			*/
					/* Implementation function	*/
	int		(*cmd)(struct cmd_tbl_s *,int, char *[]);
	char		*usage;		/* Usage message	(short)	*/
	char		*help;		/* Help  message	(long)	*/
}cmd_tbl_t;


/*
extern cmd_tbl_t  __boot_cmd_start;
extern cmd_tbl_t  __boot_cmd_end;
*/

/*
 * Monitor Command
 *
 * All commands use a common argument format:
 *
 * void function (cmd_tbl_t *cmdtp,int argc, char *argv[]);
 */

typedef	void 	command_t (cmd_tbl_t *,int, char *[]);

#endif	/* __ASSEMBLY__ */

/*
 * Configurable monitor commands definitions have been moved
 * to include/cmd_confdefs.h
 */

#define Struct_Section_CMD  __attribute__ ((section (".boot_cmd")))


#define BOOT_CMD(new_name,new_cmd,new_usage,new_help) \
cmd_tbl_t __boot_cmd_##new_name Struct_Section_CMD = {\
.name   =   #new_name,\
.cmd    =   new_cmd,\
.usage  =   new_usage,\
.help   =   new_help,\
    }
    
int do_about(cmd_tbl_t *cmdtp,int argc, char *argv[]);
int do_help(cmd_tbl_t *cmdtp,int argc,char* argv[]);
int do_memfill(cmd_tbl_t *cmdtp,int argc,char* argv[]);
int do_mcmp(cmd_tbl_t *cmdtp,int argc,char* argv[]);
int do_cp(cmd_tbl_t *cmdtp,int argc,char* argv[]);
int do_mmov(cmd_tbl_t *cmdtp,int argc,char* argv[]);
int do_md(cmd_tbl_t *cmdtp,int argc,char* argv[]);
int do_mw(cmd_tbl_t *cmdtp,int argc,char* argv[]);
int do_loadu(cmd_tbl_t *cmdtp,int argc,char * argv[]);
int do_sdramtest(cmd_tbl_t *cmdtp,int argc,char * argv[]);
int do_run(cmd_tbl_t *cmdtp,int argc,char * argv[]);
int do_selplat(cmd_tbl_t *cmdtp,int argc,char * argv[]);
int do_setup(cmd_tbl_t *cmdtp,int argc,char * argv[]);


#endif	/* __CMD_H */
