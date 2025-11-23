/* 
Автор: Сокун Михаил
Название файла: din_phil_fixed.c
Назначение программы:
Решение задачи об обедающих философах методом разрыва циклического ожидания.
Один философ (№4) берёт вилки в обратном порядке, что предотвращает дедлок.
*/

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define PHILOS 5
#define DELAY 5000
#define FOOD 50

void *philosopher (void *id);
void grab_chopstick (int, int, char *);
void down_chopsticks (int, int);
int food_on_table ();

pthread_mutex_t chopstick[PHILOS];
pthread_t philo[PHILOS];
pthread_mutex_t food_lock;

int sleep_seconds = 0;

int main(int argn, char **argv)
{
    int i;

    if (argn == 2)
        sleep_seconds = atoi(argv[1]);

    pthread_mutex_init(&food_lock, NULL);

    for (i = 0; i < PHILOS; i++)
        pthread_mutex_init(&chopstick[i], NULL);

    for (i = 0; i < PHILOS; i++)
        pthread_create(&philo[i], NULL, philosopher, (void *)i);

    for (i = 0; i < PHILOS; i++)
        pthread_join(philo[i], NULL);

    return 0;
}

void *philosopher(void *num)
{
    int id = (int)num;
    int left = (id + 1) % PHILOS;
    int right = id;
    int f;

    printf("Philosopher %d is thinking and ready to eat.\n", id);

    while ((f = food_on_table()) > 0)
    {
        /* Один философ берёт вилки в обратном порядке */
        if (id == PHILOS - 1) {
            grab_chopstick(id, left, "left");
            grab_chopstick(id, right, "right ");
        } else {
            grab_chopstick(id, right, "right ");
            grab_chopstick(id, left, "left");
        }

        printf("Philosopher %d: eating.\n", id);
        usleep(DELAY * (FOOD - f + 1));

        down_chopsticks(left, right);
    }

    printf("Philosopher %d is done eating.\n", id);
    return NULL;
}

int food_on_table()
{
    static int food = FOOD;
    int r;

    pthread_mutex_lock(&food_lock);
    if (food > 0) food--;
    r = food;
    pthread_mutex_unlock(&food_lock);

    return r;
}

void grab_chopstick(int phil, int c, char *hand)
{
    pthread_mutex_lock(&chopstick[c]);
    printf("Philosopher %d: picked up %s chopstick %d\n", phil, hand, c);
}

void down_chopsticks(int c1, int c2)
{
    pthread_mutex_unlock(&chopstick[c1]);
    pthread_mutex_unlock(&chopstick[c2]);
}


