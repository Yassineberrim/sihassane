/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yberrim <yberrim@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/08 19:24:41 by slazar            #+#    #+#             */
/*   Updated: 2023/10/11 10:19:16 by yberrim          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	only_sp_tab(char *line)
{
	while (*line && (*line == ' ' || *line == '\t'))
		line++;
	if (*line == '\0')
		return (0);
	return (1);
}
int	check_space(char *line)
{
	if (*line == '\0' || only_sp_tab(line) == 0)
		return (0);
	return (1);
}
void	join_in_quote_and_word(t_lexer *lx)
{
	t_node	*tmp;
	t_node	*cur;

	cur = lx->head;
	while (cur)
	{
		if (cur->next && ((cur->type == QOUTE || cur->type == DOUBLE_QUOTE
					|| cur->type == WORD) && (cur->next->type == QOUTE
					|| cur->next->type == DOUBLE_QUOTE
					|| cur->next->type == WORD)))
		{
			tmp = cur->next;
			cur->content = ft_strjoin(cur->content, tmp->content);
			cur->len = ft_strlen(cur->content);
			cur->next = tmp->next;
			if (tmp->next)
				tmp->next->prev = cur;
			free(tmp->content);
			free(tmp);
			lx->size -= 1;
			cur = lx->head;
		}
		cur = cur->next;
	}
}
int	ft_count_cmd(t_lexer *lx)
{
	t_node	*tmp;
	int		count;

	count = 0;
	tmp = lx->head;
	while (tmp)
	{
		if (tmp->type == PIPE_LINE)
			count++;
		tmp = tmp->next;
	}
	return (count);
}
void	init_cmd_struct(t_cmd **cmd)
{
	(*cmd)->cmd = NULL;
	(*cmd)->fd_in = 0;
	(*cmd)->fd_out = 1;
	(*cmd)->in_file = NULL;
	(*cmd)->out_file = NULL;
}

int	redirect(t_node **cur, t_cmd *cmd)
{
	if ((*cur)->type == REDIR_IN)
	{
		if (cmd->in_file)
			free(cmd->in_file);
		cmd->in_file = ft_strdup((*cur)->next->content);
		cmd->in_redir_type = READIN;
	}
	else if ((*cur)->type == REDIR_OUT)
	{
		if (cmd->out_file)
			free(cmd->out_file);
		cmd->out_file = ft_strdup((*cur)->next->content);
		cmd->out_redir_type = WRITEOUT;
	}
	else if ((*cur)->type == D_REDIR_OUT)
	{
		if (cmd->out_file)
			free(cmd->out_file);
		cmd->out_file = ft_strdup((*cur)->next->content);
		cmd->out_redir_type = APPENDOUT;
	}
	(*cur) = (*cur)->next->next;
	return (1);
}

int	check_next_next(t_node *cur)
{
	int	tmp_fd;

	if (cur->next->next && (cur->next->next->type == REDIR_IN
			|| cur->next->next->type == REDIR_OUT
			|| cur->next->next->type == D_REDIR_OUT))
	{
		tmp_fd = open(cur->next->content, O_CREAT | O_WRONLY | O_APPEND, 0644);
		close(tmp_fd);
	}
	return (1);
}

int here_doc(t_node **cur, t_cmd **cmd)
{
	int		fd;
	char	*line;
	char	*tmp;

	fd = open("/tmp/heredoc", O_CREAT | O_WRONLY | O_TRUNC, 0644);
	while (1)
	{
		line = readline(">");
		if (ft_strcmp(line, (*cur)->next->content) == 0)
		{
			free(line);
			break ;
		}
		tmp = ft_strjoin(line, "\n");
		write(fd, tmp, ft_strlen(tmp));
		free(tmp);
	}
	close(fd);
	(*cmd)->in_file = ft_strdup("/tmp/heredoc");
	(*cmd)->in_redir_type = HEREDOC;
	(*cur) = (*cur)->next->next;
	return (1);
}

void treat_cmd(t_node *cur, t_cmd *cmd, int i, int j)
{
	while (cur)
	{
		if (cur->type == HERE_DOC && here_doc(&cur, &cmd))
			continue ;
		if ((cur->type == REDIR_IN || cur->type == REDIR_OUT
				|| cur->type == D_REDIR_OUT)
			&& check_next_next(cur) && redirect(&cur, cmd))
			continue ;
		if (cur->type == PIPE_LINE && ++i)
		{
			cmd->cmd = ft_calloc(1, sizeof(char *) * (j + 1));
			cmd->next = ft_calloc(1, sizeof(t_cmd));
			cmd = cmd->next;
			cmd->fd_out = 1;
			j = 0;
		}
		else
			j++;
		cur = cur->next;
	}
	cmd->cmd = ft_calloc(sizeof(char *), (j + 1));
}

t_node	*create_cmd(t_lexer *lx, t_cmd *cmd)
{
	t_node	*cur;

	cur = lx->head;
	cmd->fd_out = 1;
	treat_cmd(cur, cmd, 0, 0);
	return (lx->head);
}

void treat_commands(t_node *cur, t_cmd *cmd)
{
	int	i;
	int	j;

	i = 0;
	j = 0;
	while (cur)
	{
		if (cur->type == PIPE_LINE && ++i)
		{
			cmd->cmd[j] = NULL;
			j = 0;
			cmd = cmd->next;
		}
		else if (cur->type == REDIR_IN || cur->type == REDIR_OUT
			|| cur->type == D_REDIR_OUT || cur->type == HERE_DOC
			|| (cur->type == WORD && cur->prev
				&& (cur->prev->type == REDIR_IN || cur->prev->type == REDIR_OUT
					|| cur->prev->type == D_REDIR_OUT
					|| cur->prev->type == HERE_DOC)))
			cur = cur->next;
		else
			cmd->cmd[j++] = ft_strdup(cur->content);
		cur = cur->next;
	}
}

t_cmd	*commands(t_lexer *lx)
{
	t_node	*cur;
	t_cmd	*cmd;
	t_cmd	*head;

	cmd = ft_calloc(sizeof(t_cmd), (ft_count_cmd(lx) + 1));
	head = cmd;
	cur = create_cmd(lx, cmd);
	treat_commands(cur, cmd);
	return (head);
}

void	print_argv(char **argv)
{
	int	i;

	i = 0;
	while (argv[i])
	{
		printf("%s", argv[i]);
		if (argv[i + 1])
			printf(", ");
		i++;
	}
	printf("\n");
}

char	*print_redir_token(out_redirs out, in_redirs in)
{
	if (out == WRITEOUT)
		return (">");
	if (out == APPENDOUT)
		return (">>");
	if (in == READIN)
		return ("<");
	if (in == HEREDOC)
		return ("<<");
	return (NULL);
}

void	print_list(t_cmd *cmd)
{
	while (cmd)
	{
		printf("argv: ");
		print_argv(cmd->cmd);
		printf("fd_in: %i\n", cmd->fd_in);
		printf("fd_out: %i\n", cmd->fd_out);
		printf("input redirections: %s\n", print_redir_token(0,
				cmd->in_redir_type));
		printf("output redirections: %s\n",
			print_redir_token(cmd->out_redir_type, 0));
		// if (cmd->in_file)
		printf("input file: %s\n", cmd->in_file);
		// if (cmd->out_file)
		printf("output file: %s\n", cmd->out_file);
		if (cmd->next)
			printf("=======================\n");
		cmd = cmd->next;
	}
}

void	sig_handler(int i)
{
	if (i == SIGINT)
	{
		printf("\n");
		rl_initialize();
		rl_on_new_line();
		// rl_replace_line("", 0);
		rl_redisplay();
	}
}
void	setup_signal_handlers(void)
{
	struct sigaction	sa;

	signal(SIGINT, sig_handler);
	sa.sa_handler = sig_handler;
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	signal(SIGQUIT, SIG_IGN);
}

void ft_free_array(char **array)
{
	int i;

	i = 0;
	while (array[i])
	{
		free(array[i]);
		i++;
	}
	if (array)
		free(array);
}

void    destroy_cmd(t_cmd *cmd)
{
	t_cmd    *tmp;

	while (cmd)
	{
		tmp = cmd->next;
		ft_free_array(cmd->cmd);
		if (cmd->in_file)
			free(cmd->in_file);
		if (cmd->out_file)
			free(cmd->out_file);
		free(cmd);
		cmd = tmp;
	}
}

int destroy_t_node(t_lexer *lx)
{
	t_node *tmp;
	t_node *cur;

	cur = lx->head;
	while (cur)
	{
		tmp = cur->next;
		if(cur->content)
			free(cur->content);
		free(cur);
		cur = tmp;
	}
	lx->head = NULL;
	lx->tail = NULL;
	return (1);
}	

int access_check(t_cmd *cmd)
{
	t_cmd	*cur;

	cur = cmd;
	while (cur)
	{
		if ((cur->in_redir_type == APPENDOUT || cur->in_redir_type == WRITEOUT)
			&& access(cur->in_file, F_OK) == -1)
			return (printf("minishell: %s: No such file or directory\n",
					cur->in_file));
		else if (cur->in_redir_type == READIN
			&& access(cur->in_file, F_OK) == -1)
			return (printf("minishell: %s: No such file or directory\n",
					cur->in_file));
		cur = cur->next;
	}
	return (0);
}

t_cmd *lexer_to_cmd(t_lexer *lx)
{
	join_quotes(lx);
	join_in_quote_and_word(lx);
	delete_white_space(lx);
	return (commands(lx));
}

void executtion(t_cmd *cmd, char **str, t_env *env)
{
	cmd->env = env;
	str = lincke_list_toaraay(env);
	execution_proto(cmd, str);
	free_double(str);
	destroy_cmd(cmd);
}


char *get_line(t_lexer *lx)
{
	char *line;

	ft_initialisation(lx);
	line = readline("minishell $-> ");
	if (!line)
		exit (1);
	add_history(line);
	return (line);
}
int free_if_line(char *line)
{
	if (line)
		free(line);
	return (1);
}
void loop(t_lexer *lx, t_env *env)
{
	char	*line;
	char 	**str;
	t_cmd	*cmd;

	cmd = NULL;
	str = NULL;
	while (1)
	{
		line = get_line(lx);
		if (check_space(line) == 0 && free_if_line(line))
		 	continue;
		if (lexer(line, lx, env) && destroy_t_node(lx) && free_if_line(line))
			continue ;
		cmd = lexer_to_cmd(lx);
		if (access_check(cmd))
		{
			destroy_cmd(cmd);
			continue ;
		}
		executtion(cmd, str, env);
		destroy_t_node(lx);
		if (line)
			free(line);
	}
}


int	main(int __unused ac, char __unused **av, char **envirement)
{
	t_env	*env;
	t_lexer	lx;

	ft_variables(&env, envirement);
	setup_signal_handlers();
	loop(&lx, env);
	return (0);
}
