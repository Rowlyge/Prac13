/* 
Автор: Сокун Михаил
Название файла: pr13.go
Назначение программы: 
Программа реализует многопоточное приложение на Go. 
Первый поток генерирует каждую секунду случайное целое число.
Если число чётное — второй поток вычисляет его квадрат и печатает.
Если число нечётное — третий поток вычисляет куб числа и печатает.
*/

package main

import (
	"fmt"
	"math/rand"
	"time"
)

func main() {
	numberChan := make(chan int)
	evenChan := make(chan int)
	oddChan := make(chan int)

	// Генерация случайных чисел
	go func() {
		for {
			n := rand.Intn(100)
			fmt.Println("Сгенерировано число:", n)
			numberChan <- n
			time.Sleep(1 * time.Second)
		}
	}()

	// Поток обработки чётных чисел
	go func() {
		for num := range evenChan {
			fmt.Printf("Чётное число %d, квадрат = %d\n", num, num*num)
		}
	}()

	// Поток обработки нечётных чисел
	go func() {
		for num := range oddChan {
			fmt.Printf("Нечётное число %d, куб = %d\n", num, num*num*num)
		}
	}()

	// Маршрутизация чисел
	for {
		num := <-numberChan
		if num%2 == 0 {
			evenChan <- num
		} else {
			oddChan <- num
		}
	}
}

