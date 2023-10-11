/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   find_path.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yberrim <yberrim@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/02 16:18:37 by yberrim           #+#    #+#             */
/*   Updated: 2023/10/11 00:39:37 by yberrim          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

void	free_double(char **str)
{
	int	i;

	i = 0;
	if (!str)
		return ;
	while (str[i])
	{
		free(str[i]);
		i++;
	}
	free(str);
	str = NULL;
}

int	chek_path(char *cmd)
{
	if (cmd && cmd[0] == '/' && ft_strlen(cmd) > 1)
	{
		if (access(cmd, F_OK) == 0)
			return (1);
		return (0);
	}
	if (cmd && cmd[0] == '.' && cmd[1] == '/' && ft_strlen(cmd) > 2)
	{
		if (access(cmd, F_OK) == 0)
			return (1);
		return (0);
	}
	return (-1);
}

char	*find_abs_path(t_env *env, char *cmd)
{
	t_cmd	c;

	c.i = 0;
	c.raw_path = ft_genv(env, "PATH");
	c.path_arr = ft_split(c.raw_path, ':');
	if (chek_path(cmd) == 1)
	{
		free_double(c.path_arr);
		return (cmd);
	}
	while (c.path_arr && c.path_arr[c.i])
	{
		c.fwd_slash = ft_strjoin(ft_strdup(c.path_arr[c.i]), "/");
		c.abs_path = ft_strjoin(c.fwd_slash, cmd);
		if (access(c.abs_path, F_OK) == 0)
		{
			free_double(c.path_arr);
			return (c.abs_path);
		}
		free(c.abs_path);
		c.i++;
	}
	free_double(c.path_arr);
	return (NULL);
}
