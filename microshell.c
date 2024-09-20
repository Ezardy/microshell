#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

size_t	ft_strlen(const char *str);
char	**pipe_or_semicolon(char **argv);
void	print_error_fatal(void);
void	print_error_bad_arguments(void);
void	print_error_execve(char *str);
bool	mdup(int src, int *dst, int *error);
bool	mdup2(int *src, int dst, int *error);
bool	mpipe(int fds[2], int *error);
bool	mclose(int *fd, int *error);

int	main(int argc, char **argv, char **envp)
{
	int		error;
	int		fdout;
	int		fdin;
	int		fds[2];
	pid_t	pid;
	char	**delim;
	int		status;

	error = (strcmp(argv[argc - 1], "|") == 0) * 300;
	argv += 1;
	fds[0] = -1;
	fds[1] = -1;
	fdin = -1;
	fdout = -1;
	while (*argv && error < 258)
	{
		if (!strcmp(*argv, "cd"))
		{
			argv += 1;
			if (!*argv || !strcmp(*argv, "|") || !strcmp(*argv, ";")
				|| (argv[1] && strcmp(argv[1], "|") && strcmp(argv[1], ";")))
			{
				error = 256;
				print_error_bad_arguments();
			}
			else
			{
				error = (chdir(*argv) == -1) * 257;
				if (error)
				{
					write(2, "error: cd: cannot change directory to ", 38);
					write(2, *argv, ft_strlen(*argv));
					write(2, "\n", 1);
				}
				argv += 1;
			}
		}
		if (*argv)
		{
			mdup(0, &fdin, &error);
			mdup(1, &fdout, &error);
			if (error < 258)
				delim = pipe_or_semicolon(argv);
			while (error < 258 && *delim && !strcmp(*delim, "|") && delim[1])
			{
				mpipe(fds, &error);
				pid = fork();
				if (!pid)
				{
					mclose(&fdin, &error);
					mclose(&fdout, &error);
					mclose(fds, &error);
					mdup2(fds + 1, 1, &error);
					if (error < 258)
					{
						*pipe_or_semicolon(argv) = NULL;
						error = execve(argv[0], argv, envp);
					}
					if (error == -1)
						print_error_execve(argv[0]);
					else if (error < 258)
						print_error_fatal();
					exit(3);
				}
				else if (pid == -1)
					error = 259;
				else
				{
					mclose(fds + 1, &error);
					mdup2(fds, 0, &error);
					argv = delim + 1;
					delim = pipe_or_semicolon(argv);
				}
			}
			if (error < 258 && *argv && strcmp(*argv, ";"))
			{
				mdup2(&fdout, 1, &error);
				if (error < 258)
				{
					pid = fork();
					if (!pid)
					{
						mclose(&fdin, &error);
						if (error < 258)
						{
							*delim = NULL;
							error = execve(argv[0], argv, envp);
						}
						if (error == -1)
							print_error_execve(argv[0]);
						else if (error < 258)
							print_error_fatal();
						exit(3);
					}
					else if (pid == -1)
						error = 259;
					else
					{
						mdup2(&fdin, 0, &error);
						argv = delim;
					}
				}
			}
			if (error < 258)
			{
				pid = waitpid(-1, &status, 0);
				while (pid != -1 || errno == EINTR)
				{
					if (pid != -1 && WIFEXITED(status))
						error = WEXITSTATUS(status);
					pid = waitpid(-1, &status, 0);
				}
			}
		}
		if (error < 258 && *argv && (!strcmp(*argv, ";") || !strcmp(*argv, "|")))
			argv += 1;
		mclose(fds, &error);
		mclose(fds + 1, &error);
		mclose(&fdin, &error);
		mclose(&fdout, &error);
	}
	if (error > 258 && error != 300)
		print_error_fatal();
	if (error > 255)
		error = 1;
	return (error);
}

bool	mpipe(int fds[2], int *error)
{
	if (*error < 258)
		*error = (pipe(fds) == -1) * 259;
	return (!*error);
}

bool	mdup2(int *src, int dst, int *error)
{
	if (*error < 258)
	{
		*error = (dup2(*src, dst) == -1) * 259;
		mclose(src, error);
	}
	return (!*error);
}

bool	mclose(int *fd, int *error)
{
	if (*fd != -1)
	{
		if (close(*fd) == -1)
		{
			if (*error < 258)
				*error = 259;
		}
		else
			*fd = -1;
	}
	return (*fd == -1);
}

bool	mdup(int src, int *dst, int *error)
{
	if (*error < 258)
	{
		*dst = dup(src);
		*error = (*dst == -1) * 259;
	}
	return (!*error);
}

size_t	ft_strlen(const char *str)
{
	size_t	i;

	i = 0;
	while (*str++)
		i += 1;
	return (i);
}

char	**pipe_or_semicolon(char **argv)
{
	while (*argv && strcmp(*argv, "|") && strcmp(*argv, ";"))
		argv += 1;
	return (argv);
}

void	print_error_fatal(void)
{
	write(2, "error: fatal\n", 13);
}

void	print_error_bad_arguments(void)
{
	write(2, "error: cd: bad arguments\n", 25);
}

void	print_error_execve(char *str)
{
	write(2, "error: cannot execute ", 22);
	write(2, str, ft_strlen(str));
	write(2, "\n", 1);
}
