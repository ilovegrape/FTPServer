#include "command.h"

char* cmd_strings[] = {"USER", "PASS", "SYST", "PASV", "RETR", "QUIT", "PORT"};






FTP_CMD command_parser(char* buffer)
{
	int i;
	for(i=0; i<sizeof(cmd_strings)/sizeof(cmd_strings[0]);++i)
	{
		if(strncmp(buffer, cmd_strings[i], 4) == 0)
		{
			break;
		}
	}
	return (CMD_USER + i);
}

