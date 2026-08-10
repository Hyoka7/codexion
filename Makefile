NAME = codexion

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread

SRCS = cmp.c \
	   coder_routine.c \
       coders.c \
	   dongle_atomic.c \
	   dongle_common.c \
	   dongle_get.c \
	   dongle_init.c \
	   dongle_release.c \
	   dongle_single.c \
	   dongle_wait.c \
	   log.c \
       main.c \
       mainloop.c \
       monitor.c \
       parse.c \
       priority_queue_1.c \
       priority_queue_2.c \
	   sim_init.c \
       time.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c codexion.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
