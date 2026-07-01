#include "codexion.h"
#include <limits.h>

static int ft_isspace(char ch) {
  if ((9 <= ch && ch <= 13) || ch == 32)
    return (1);
  return (0);
}

static int ft_isdigit(char ch) {
  if ('0' <= ch && ch <= '9')
    return (1);
  return (0);
}

static int ft_atoi(char *nbr, int *err_flag) {
  long num;
  long sign;

  num = 0;
  sign = -1;
  while (*nbr && ft_isspace(*nbr))
    nbr++;
  if (*nbr && (*nbr == '+' || *nbr == '-')) {
    if (*nbr == '-')
      sign = 1;
    nbr++;
  }
  while (*nbr && ft_isdigit(*nbr)) {
    num = num * 10 - (*nbr - '0');
    if ((sign == 1 && num < INT_MIN) || (sign == -1 && num < -INT_MAX)) {
      *err_flag = 1;
      return (-1);
    }
  }
  return ((int)num * sign);
}

int *parser(char **argv)
{
  int *parse_res;
  int arg_i;
  int atoi_res;
  int err_flag;

  parse_res = (int *)malloc(sizeof(int) * CONFIG_MAX);
  if (!parse_res)
    return (NULL);
  arg_i = 0;
  err_flag = 0;
  while (arg_i < CONFIG_MAX)
  {
    atoi_res = ft_atoi(argv[arg_i], &err_flag);
    if (err_flag || atoi_res < 0)
    {
      free(parse_res);
      return (NULL);
    }
    parse_res[arg_i] = atoi_res;
    arg_i++;
  }
  return (parse_res);
}

