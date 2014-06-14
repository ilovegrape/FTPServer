#ifndef __COMMAND_H__
#define __COMMAND_H__

typedef enum {
	CMD_USER = 0,
	CMD_PASS,
	CMD_SYST,
	CMD_PASV,
	CMD_RETR,
	CMD_QUIT,
	CMD_PORT,

	CMD_MAX
}FTP_CMD;

//extern char* cmd_strings[];

FTP_CMD command_parser(char*);

#endif //!__COMMAND_H__

