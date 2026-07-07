/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/01 20:48:18 by hfujisad          #+#    #+#             */
/*   Updated: 2026/07/07 18:31:52 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <limits.h>

static int	ft_isspace(char ch)
{
	if ((9 <= ch && ch <= 13) || ch == 32)
		return (1);
	return (0);
}

static int	ft_isdigit(char ch)
{
	if ('0' <= ch && ch <= '9')
		return (1);
	return (0);
}

static int	ft_atoi(char *nbr, int *err_flag)
{
	long	num;
	long	sign;

	num = 0;
	sign = -1;
	while (*nbr && ft_isspace(*nbr))
		nbr++;
	if (*nbr && (*nbr == '+' || *nbr == '-'))
	{
		if (*nbr == '-')
			sign = 1;
		nbr++;
	}
	while (*nbr && ft_isdigit(*nbr))
	{
		num = num * 10 - (*nbr - '0');
		if ((sign == 1 && num < INT_MIN) || (sign == -1 && num < -INT_MAX))
		{
			*err_flag = 1;
			return (-1);
		}
		nbr++;
	}
	return ((int)num * sign);
}

int	*parser(char **argv, char **scheduler)
{
	int	*parse_res;
	int	arg_i;
	int	atoi_res;
	int	err_flag;

	parse_res = (int *)malloc(sizeof(int) * CONFIG_MAX);
	if (!parse_res)
		return (NULL);
	arg_i = 1;
	err_flag = 0;
	while (arg_i <= CONFIG_MAX)
	{
		atoi_res = ft_atoi(argv[arg_i], &err_flag);
		if (err_flag || atoi_res < 0)
			return (free(parse_res), NULL);
		parse_res[arg_i - 1] = atoi_res;
		arg_i++;
	}
	if (strcmp(argv[arg_i], "fifo") != 0 && strcmp(argv[arg_i], "edf") != 0)
		return (free(parse_res), NULL);
	*scheduler = argv[arg_i];
	return (parse_res);
}
