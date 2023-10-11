/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exex_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yberrim <yberrim@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/11 01:01:51 by yberrim           #+#    #+#             */
/*   Updated: 2023/10/11 01:53:01 by yberrim          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

int	built_in(t_cmd *cmd)
{
	if (is_buildin(cmd))
	{
		execution_builtin(cmd, 0);
		return (g_exit_status);
	}
	else if (!cmd->cmd_path)
	{
		ft_putstr_fd("minishell: command not found\n", 2);
		return (g_exit_status = 127);
	}
	return (-1);
}

int	exec(t_cmd *cmd, char **env, int **pipe_fd)
{
	if (cmd->fd_out != 1)
	{
		dup2(cmd->fd_out, 1);
		close(cmd->fd_out);
	}
	if (cmd->fd_in != 0)
	{
		dup2(cmd->fd_in, 0);
		close(cmd->fd_in);
	}
	if (cmd->out_redir_type != IN_NONE || cmd->in_redir_type != OUT_NONE)
	{
		dup2(*pipe_fd[1], 1);
		close(*pipe_fd[1]);
	}
	close(*pipe_fd[0]);
	close(*pipe_fd[1]);
	(void)env;
	execve(cmd->cmd_path, cmd->cmd, lincke_list_toaraay(cmd->env));
	exit(g_exit_status = 127);
	return (g_exit_status = 127);
}

void	ft_close(t_cmd *cmd, int **pipe_fd)
{
	if (cmd->out_redir_type != IN_NONE || cmd->in_redir_type != OUT_NONE)
		close(*pipe_fd[1]);
	if (cmd->fd_out != 1)
		close(cmd->fd_out);
	if (cmd->fd_in != 0)
		close(cmd->fd_in);
}

void	execute_command_v1(t_cmd *cmd, char **env)
{
	int		*pipe_fd;
	t_cmd	*head;

	head = cmd;
	pipe_fd = malloc(sizeof(int) * 2);
	pipe(pipe_fd);
	check_redirections(cmd);
	cmd->cmd_path = find_abs_path(head->env, cmd->cmd[0]);
	if (cmd->cmd_path == NULL)
	{
		ft_putstr_fd("minishell: command not found\n", 2);
		exit(127);
	}
	exec(cmd, env, &pipe_fd);
	free(pipe_fd);
	exit(g_exit_status);
}
