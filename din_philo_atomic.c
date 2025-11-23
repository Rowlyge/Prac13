/* 
Автор: Сокун Михаил
Название файла: din_philo_atomic.c
Назначение программы:
Решение задачи об обедающих философах с атомарным (логическим) захватом вилок.
Философ пытается взять обе вилки через pthread_mutex_trylock; если взять обе нельзя,
он освобождает любую захваченную вилку и ждёт на условной переменной.
*/

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define PHILOS 5
#define DELAY 5000
#define FOOD 50

void *philosopher(void *id);
void pickup_chopsticks_atomic(int phil, int left, int right);
void putdown_chopsticks_atomic(int left, int right);
int food_on_table(void);

pthread_mutex_t chopstick[PHILOS];
pthread_t philo[PHILOS];
pthread_mutex_t food_lock;

/* Мьютекс и условная переменная для координации попыток взять вилки */
pthread_mutex_t forks_lock;
pthread_cond_t  forks_cond;

int sleep_seconds = 0;

int main(int argc, char **argv)
{
    int i;

    if (argc == 2)
        sleep_seconds = atoi(argv[1]);

    pthread_mutex_init(&food_lock, NULL);
    pthread_mutex_init(&forks_lock, NULL);
    pthread_cond_init(&forks_cond, NULL);

    for (i = 0; i < PHILOS; i++)
        pthread_mutex_init(&chopstick[i], NULL);

    for (i = 0; i < PHILOS; i++)
        pthread_create(&philo[i], NULL, philosopher, (void *)(long)i);

    for (i = 0; i < PHILOS; i++)
        pthread_join(philo[i], NULL);

    /* Очистка (опционально) */
    for (i = 0; i < PHILOS; i++)
        pthread_mutex_destroy(&chopstick[i]);
    pthread_mutex_destroy(&food_lock);
    pthread_mutex_destroy(&forks_lock);
    pthread_cond_destroy(&forks_cond);

    return 0;
}

void *philosopher(void *num)
{
    int id = (int)(long)num;
    int left = (id + 1) % PHILOS;
    int right = id;
    int f;

    printf("Philosopher %d is thinking and ready to eat.\n", id);

    while ((f = food_on_table()) > 0) {
        /* Сохраняем прежнюю опциональную рассинхронизацию поведения */
        if (id == 1)
            sleep(sleep_seconds);

        /* Попытка атомарно взять обе вилки */
        pickup_chopsticks_atomic(id, left, right);

        /* Еда */
        printf("Philosopher %d: eating.\n", id);
        usleep(DELAY * (FOOD - f + 1));

        /* Кладём вилки обратно и оповещаем */
        putdown_chopsticks_atomic(left, right);
    }

    printf("Philosopher %d is done eating.\n", id);
    return NULL;
}

/* Безопасная функция доступа к общему ресурсу "еда" */
int food_on_table(void)
{
    static int food = FOOD;
    int r;

    pthread_mutex_lock(&food_lock);
    if (food > 0) food--;
    r = food;
    pthread_mutex_unlock(&food_lock);

    return r;
}

/*
 * Алгоритм pickup_chopsticks_atomic:
 * - Захватываем forks_lock, чтобы серийно проверять и изменять состояние вилок.
 * - Пробуем взять обе вилки через pthread_mutex_trylock.
 * - Если обе взяты — отпускаем forks_lock и выходим.
 * - Если взяли только одну или ни одной — освобождаем захваченные (если были)
 *   и ждем на условной переменной forks_cond (под forks_lock).
 * - При пробуждении повторяем попытку (защита от spurious wakeups).
 */
void pickup_chopsticks_atomic(int phil, int left, int right)
{
    int left_acquired, right_acquired;

    pthread_mutex_lock(&forks_lock);
    for (;;) {
        left_acquired  = (pthread_mutex_trylock(&chopstick[left])  == 0);
        right_acquired = (pthread_mutex_trylock(&chopstick[right]) == 0);

        if (left_acquired && right_acquired) {
            /* Успешно взял обе вилки */
            pthread_mutex_unlock(&forks_lock);
            printf("Philosopher %d: got left %d and right %d\n", phil, left, right);
            return;
        }

        /* Если захвачена только одна — вернуть её */
        if (left_acquired) {
            pthread_mutex_unlock(&chopstick[left]);
            left_acquired = 0;
        }
        if (right_acquired) {
            pthread_mutex_unlock(&chopstick[right]);
            right_acquired = 0;
        }

        /* Ждём оповещения об освобождении вилок. Ждём под forks_lock — это
           предотвращает потерянное пробуждение: сигнал делается тоже под forks_lock. */
        pthread_cond_wait(&forks_cond, &forks_lock);
        /* После пробуждения повторяем попытку (защита от spurious wakeups) */
    }
}

/*
 * putdown_chopsticks_atomic:
 * - Производим возврат обеих вилок (unlock).
 * - Посылаем pthread_cond_broadcast под forks_lock для пробуждения всех ожидающих.
 * Делать это под forks_lock важно, чтобы избежать гонки между освобождением и ожиданием.
 */
void putdown_chopsticks_atomic(int left, int right)
{
    pthread_mutex_lock(&forks_lock);

    /* Освобождаем вилки, которые гарантированно захвачены владельцем */
    pthread_mutex_unlock(&chopstick[left]);
    pthread_mutex_unlock(&chopstick[right]);

    /* Оповещаем всех ожидающих философов */
    pthread_cond_broadcast(&forks_cond);

    pthread_mutex_unlock(&forks_lock);
}

