/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execution.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yberrim <yberrim@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/14 17:43:17 by yberrim           #+#    #+#             */
/*   Updated: 2023/10/11 01:56:48 by yberrim          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

void	setup_pipe(t_cmd *cmd, int *pipe_fd)
{
	if (cmd && cmd->next)
	{
		pipe(pipe_fd);
		cmd->fd_out = pipe_fd[1];
		cmd->next->fd_in = pipe_fd[0];
	}
}

void	execute_command_v2(t_cmd *cmd, char **env)
{
	int		*pipe_fd;
	int		j;
	t_cmd	*head;

	j = 0;
	head = cmd;
	while (cmd)
	{
		pipe_fd = malloc(sizeof(int) * 2);
		if (is_buildin(cmd) == 0)
			cmd->cmd_path = find_abs_path(head->env, cmd->cmd[0]);
		setup_pipe(cmd, pipe_fd);
		check_redirections(cmd);
		if (!((is_buildin(cmd)) && built_in(cmd) != -1))
		{
			cmd->child_pid = fork();
			if (cmd->child_pid == 0)
				exec(cmd, env, &pipe_fd);
		}
		free(cmd->cmd_path);
		ft_close(cmd, &pipe_fd);
		free(pipe_fd);
		cmd = cmd->next;
	}
}

void	dup_chek(t_cmd *cmd)
{
	int	backup_output;
	int	backup_input;

	backup_output = dup(1);
	backup_input = dup(0);
	check_redirections(cmd);
	built_in(cmd);
	if (cmd->fd_out != 1)
		dup2(backup_output, 1);
	if (cmd->fd_in != 0)
		dup2(backup_input, 0);
}

int	return_stau(void)
{
	while (wait(&g_exit_status) > 0)
	{
		if (WIFEXITED(g_exit_status))
			g_exit_status = WEXITSTATUS(g_exit_status);
		else if (WIFSIGNALED(g_exit_status))
			g_exit_status = WTERMSIG(g_exit_status) + 128;
	}
	return (g_exit_status);
}

int	execution_proto(t_cmd *cmd, char **env)
{
	int		j;
	t_cmd	*head;

	j = 0;
	head = cmd;
	if (!cmd->next)
	{
		if (is_buildin(cmd))
			dup_chek(cmd);
		else
		{
			cmd->child_pid = fork();
			if (cmd->child_pid == 0)
				execute_command_v1(cmd, env);
			waitpid(cmd->child_pid, &g_exit_status, 0);
			if (WIFEXITED(g_exit_status))
				g_exit_status = WEXITSTATUS(g_exit_status);
			else if (WIFSIGNALED(g_exit_status))
				g_exit_status = WTERMSIG(g_exit_status) + 128;
		}
		return (g_exit_status);
	}
	execute_command_v2(cmd, env);
	return (return_stau());
}
