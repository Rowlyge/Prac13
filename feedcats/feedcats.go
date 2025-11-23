/* 
Автор: Сокун Михаил
Название файла: feedcats.go
Назначение программы:
Многопоточное приложение «Покорми кота». 
Владелец поддерживает уровень корма в кормушке в пределах 100–1000 грамм.
Пять котов питаются одновременно, каждый съедает по 100 грамм в день (1 секунда).
Если корма становится меньше минимального уровня, владелец пополняет кормушку.
*/

package main

import (
	"fmt"
	"sync"
	"time"
)

const (
	MaxFood     = 1000 // максимум в кормушке
	MinFood     = 100  // минимум для пополнения
	FoodPerCat  = 100  // кот съедает 100 г в день
	CatsCount   = 5
	CheckPeriod = 1 * time.Second
)

var food int = 500
var mu sync.Mutex

func owner() {
	for {
		time.Sleep(CheckPeriod)

		mu.Lock()
		if food < MinFood {
			fmt.Println("Владелец подсыпает корм до максимума...")
			food = MaxFood
			fmt.Printf("Кормушка пополнена до %d г.\n", food)
		}
		mu.Unlock()
	}
}

func cat(id int) {
	for {
		time.Sleep(CheckPeriod)

		mu.Lock()
		if food >= FoodPerCat {
			food -= FoodPerCat
			fmt.Printf("Кот #%d поел 100 г. Осталось: %d г\n", id, food)
		} else {
			fmt.Printf("Кот #%d не смог поесть — мало корма (%d г)\n", id, food)
		}
		mu.Unlock()
	}
}

func main() {
	fmt.Println("Старт программы «Покорми кота»")
	fmt.Printf("Начальное количество корма: %d г\n\n", food)

	go owner()

	for i := 1; i <= CatsCount; i++ {
		go cat(i)
	}

	select {} // бесконечная блокировка
}

