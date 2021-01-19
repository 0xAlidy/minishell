/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_pwd.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alidy <alidy@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/01/19 08:47:58 by alidy             #+#    #+#             */
/*   Updated: 2021/01/19 08:48:13 by alidy            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

void    ft_pwd(m_sct *sct)
{
    char    *path;

    if(!(path = malloc(PATH_MAX * sizeof(char))))
    {
        ft_printf("[ERROR] MALLOC");
        exit(EXIT_FAILURE);
    }
    if (sct->args[0])
        ft_printf("pwd: too many arguments\n");
    else
        ft_printf("%s\n", getcwd(path, PATH_MAX));
}