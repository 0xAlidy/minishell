/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strfree.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alidy <alidy@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/01/19 17:19:11 by alidy             #+#    #+#             */
/*   Updated: 2021/01/27 15:59:36 by alidy            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

void	ft_strfree(char *s1, char *s2, int fr)
{
	if (fr == 1 && s1)
	{
		free(s1);
		s1 = 0;
	}
	else if (fr == 2 && s2)
	{
		free(s2);
		s2 = 0;
	}
	else if (fr == 3 && (s1 || s2))
	{
		if (s1)
		{
			free(s1);
			s1 = 0;
		}
		if (s2)
		{
			free(s2);
			s2 = 0;
		}
	}
}
