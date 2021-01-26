/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ms_exec.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alidy <alidy@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/01/16 10:10:45 by alidy             #+#    #+#             */
/*   Updated: 2021/01/26 11:32:47 by alidy            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

int     ms_test_path(char *path)
{
    struct stat buf;
    
    if (stat(path, &buf) == 0)
    {
            if (S_ISDIR(buf.st_mode))
            {
                ft_printf("%s: is a directory\n", path); 
                return (-1);
            }
            if (S_ISREG(buf.st_mode))
                return TRUE;
    }
    return FALSE;
}
char   *ms_search_path(m_sct *sct)
{
    int i;
    int save;
    char *current_path;
    
    i = 0;
    save = 0;
    current_path = 0;
    while (sct->path[i])
    {
        save = i;
        while (sct->path[i] && sct->path[i] != ':') 
            i++;
        current_path = ft_substr(sct->path, save, i - save);
        if (sct->path[i - 1] != '/')
            current_path = ft_strjoin_free(current_path, "/", 1);
        current_path = ft_strjoin_free(current_path, sct->args[0], 1);
        if (ms_test_path(current_path) != TRUE)
            free(current_path);
        else
        {
            free(sct->path);
            return (current_path);
        }
        if (sct->path[i])
            i++;
    }
    errno = ENOENT; // command not found
    return (0);
}

void    ms_reset_fd(m_sct *sct, int check)
{
    if (sct->saved_stdout != -1 && (check == 1 || check == 3))
    {
        dup2(sct->saved_stdout, STDOUT_FILENO);
        close(sct->saved_stdout);
        sct->saved_stdout = -1;
    }   
    if (sct->saved_stdin != -1 && (check == 2 || check == 3))
    {
        dup2(sct->saved_stdin, STDIN_FILENO);
        close(sct->saved_stdin);
        sct->saved_stdin = -1;
    }
}

int   ms_set_redirections(m_sct *sct, m_cmd *command)
{
    m_output    *temp;
    int         fd;

    temp = command->output;
    while (temp)
    {
        if (temp->type == 1)
        { 
            ms_reset_fd(sct, 2);
            sct->saved_stdin = dup(STDIN_FILENO);
            if ((fd = open(temp->content, O_RDONLY)) == -1)
            {
                ms_reset_fd(sct, 3);
                ft_printf("Minishell : %s: %s\n", temp->content, strerror(errno));
                sct->status = 1;
                return (-1);
            }
            dup2(fd, STDIN_FILENO); // stdin
            close(fd);
        }
        else if (temp->type == 2)
        {
            ms_reset_fd(sct, 1);
            sct->saved_stdout = dup(STDOUT_FILENO);
            if (!(fd = open(temp->content, O_WRONLY | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)))
            {
                ms_reset_fd(sct, 3);
                ft_printf("Minishell : %s: %s\n", temp->content, strerror(errno));
                sct->status = 1;
                return (-1);
            }
            dup2(fd, STDOUT_FILENO); 
            close(fd);
        }
        else
        {
            ms_reset_fd(sct, 1);
            sct->saved_stdout = dup(STDOUT_FILENO);
            if (!(fd = open(temp->content, O_WRONLY | O_CREAT | O_APPEND,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)))
            {
                ms_reset_fd(sct, 3);
                ft_printf("Minishell : %s: %s\n", temp->content, strerror(errno));
                sct->status = 1;
                return (-1);
            }
            dup2(fd, STDOUT_FILENO); // stdout
            close(fd);
        }
        temp = temp->next;
    }
    return (TRUE);
}

void    ms_set_envp(m_sct *sct, m_env **env)
{
    int     len;
    m_env   *temp;

    temp = *env;
    len = 0;
    while (temp)
    {
        len++;
        temp = temp->next;
    }
    if (!(sct->envp = malloc((len + 1) * sizeof(char *))))
        exit(EXIT_FAILURE);
    sct->envp[len] = 0;
    temp = *env;
    len = 0;
    while (temp)
    {
        sct->envp[len] = ft_strdup(temp->name);
        sct->envp[len] = ft_strjoin_free(sct->envp[len], "=", 1);
        sct->envp[len] = ft_strjoin_free(sct->envp[len], temp->content, 1);
        len++;
        temp = temp->next;
    }
}

void    ms_transform_args(m_sct *sct, m_arg **args)
{
    int     len;
    m_arg   *temp;

    temp = *args;
    len = 0;
    while (temp)
    {
        len++;
        temp = temp->next;
    }
    if (!(sct->args = malloc((len + 1) * sizeof(char *))))
        exit(EXIT_FAILURE);
    sct->args[len] = 0;
    temp = *args;
    len = 0;
    while (temp)
    {
        temp->content = ft_replace(temp->content, "$?", ft_itoa(sct->status), 1);
        temp->content = ft_replace(temp->content, "\t", "", 1);
        sct->args[len] = ft_strdup(temp->content);
        len++;
        temp = temp->next;
    }
}

void    ms_exec_fork(m_sct *sct, m_cmd *command, m_env *env)
{ 
    pid_t pid;
    
    ms_signal_handler(2);
    if ((pid = fork()) == -1)
        exit(EXIT_FAILURE);
    else if (pid == 0)
    {
        sct->in_fork = TRUE;
        ms_exec_simple_command(sct, command, env);
        exit(sct->status);
    }
    else
    {
        if (wait(&sct->status) < 0)
            exit(EXIT_FAILURE); // exit + free tout
        if (WIFEXITED(sct->status))
            sct->status = WEXITSTATUS(sct->status);
        else if (WIFSIGNALED(sct->status))
        {
            if (sct->status == 2)
                sct->status = 130;
            else if (sct->status == 3)
                sct->status = 131; 
        } 
        command = command->next;
    }
    ms_signal_handler(1);
}

int    ms_handler_builtin(m_sct *sct, m_env *env)
{
    if (!ft_strncmp(sct->args[0], "echo", 5))
        ms_echo(sct);
    else if (!ft_strncmp(sct->args[0], "pwd", 4))
        ms_pwd(sct);
    else if (!ft_strncmp(sct->args[0], "env", 4))
        ms_env(sct);
    else if (!ft_strncmp(sct->args[0], "export", 7))
        ms_export(sct, &env);
    else if (!ft_strncmp(sct->args[0], "unset", 6))
        ms_unset(sct, &env);
    else if (!ft_strncmp(sct->args[0], "cd", 3))
        ms_cd(sct, &env);
    else if (!ft_strncmp(sct->args[0], "exit", 5))
        ms_exit(sct, env);
    else
        return (FALSE);
    return (TRUE);
}

void    ms_exec_simple_command(m_sct *sct, m_cmd *command, m_env *env)
{
    ms_transform_args(sct, &(command->args));
    ms_set_envp(sct, &env);
    sct->status = 0;
    if ((ms_set_redirections(sct, command)) == -1)
    {
        ms_free_tab(sct->args);
        ms_free_tab(sct->envp);
        return;
    }
    if (strchr(sct->args[0], '/')) // si un chemin (error : No such file or directory)
    {
        if (!sct->in_pipe && !sct->in_fork)
            ms_exec_fork(sct, command, env);
        else
        {
            if (ms_test_path(sct->args[0]) == 0)
                ft_printf("%s: No such file or directory\n", sct->args[0]);
            else
                execve(sct->args[0], sct->args, sct->envp);
        }       
    }
    else if (ms_handler_builtin(sct, env)) // si un built in
    {
        ms_free_tab(sct->args);
        ms_free_tab(sct->envp);
       return;
    }
    else // sinon chercher dans l'env
    {
        if (!sct->in_pipe && !sct->in_fork)
            ms_exec_fork(sct, command, env);
        else
        {
            sct->path = ms_search_env("PATH", &env);
            sct->path = ms_search_path(sct);
            if (!sct->path)
            {
                ms_reset_fd(sct, 3);
                ft_printf("Minishell: %s: command not found\n", sct->args[0]);
                sct->status = 127;
                ms_free_tab(sct->args);
                ms_free_tab(sct->envp);
                return;
            }
            execve(sct->path, sct->args, sct->envp);
            free(sct->path);
            sct->path = 0;
        }
    }
    ms_free_tab(sct->args);
    ms_free_tab(sct->envp);
}

void    ms_exec_pipe(m_sct *sct, m_cmd **command, m_env *env)
{
    int     pipefd[2];
	pid_t   pid;
	int     save_fd;
    int     last;
    
    save_fd = 0;
    last = FALSE;
    while (*command && ((*command)->pipe || last == TRUE))
    {
        if (pipe(pipefd) == -1)
            exit(EXIT_FAILURE);
        if ((pid = fork()) == -1)
            exit(sct->status);
        else if (pid == 0)
        {
            sct->saved_stdout = dup(STDOUT_FILENO);
            dup2(save_fd, 0); //change the input according to the old one 
            if ((*command)->next)
                dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[0]);
            ms_exec_simple_command(sct, *command, env);
            exit(sct->status);
        }
        else
        {
            if (wait(&sct->status) < 0)
                exit(EXIT_FAILURE); // exit + free tout
            sct->status = 0;
            close(pipefd[1]);
            save_fd = pipefd[0]; //save the input for the next command
            if (last != TRUE)
                (*command) = (*command)->next;
            if (last == FALSE && !(*command)->pipe)
                last = TRUE;
            else
                last = FALSE;
        }
    }
}

void    ms_exec_commands(m_sct *sct, m_cmd **commands, m_env *env)
{
    m_cmd   *command;

    command = *commands;
    sct->saved_cmds = commands;
    while (command)
    {
        sct->in_fork = FALSE;
        if (command->pipe)
        {
            sct->in_pipe = TRUE;
            ms_exec_pipe(sct, &command, env);
        }
        else
        {
            sct->in_pipe = FALSE;
            ms_exec_simple_command(sct, command, env);
        }
        ms_reset_fd(sct, 3);
        if (command)
            command = command->next;
    }
}