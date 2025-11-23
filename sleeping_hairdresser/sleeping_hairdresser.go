package main

import (
	"fmt"
	"math/rand"
	"time"
)

const (
	NumSeats   = 3 // количество стульев в приёмной
	NumClients = 10
)

// Каналы
var (
	waitingRoom = make(chan int, NumSeats) // очередь клиентов
	barberReady = make(chan bool)          // сигнал парикмахеру работать
	done        = make(chan int)           // клиент закончил стрижку
)

// Парикмахер
func barber() {
	for {
		// Если клиентов нет — парикмахер "спит"
		fmt.Println("Парикмахер: сплю, клиентов нет.")
		<-barberReady // просыпается, когда клиент приходит

		// Берём следующего клиента из очереди
		clientID := <-waitingRoom
		fmt.Printf("Парикмахер: стригу клиента %d...\n", clientID)

		time.Sleep(time.Duration(rand.Intn(1000)+500) * time.Millisecond)

		fmt.Printf("Парикмахер: клиент %d подстрижен.\n", clientID)
		done <- clientID
	}
}

// Клиент
func client(id int) {
	fmt.Printf("Клиент %d пришёл в парикмахерскую.\n", id)

	// Пытаемся сесть в приёмную
	select {
	case waitingRoom <- id:
		fmt.Printf("Клиент %d сел в приёмную.\n", id)

		// Будим парикмахера
		barberReady <- true

		// Ждём окончания стрижки
		<-done
		fmt.Printf("Клиент %d уходит довольный.\n", id)

	default:
		// Мест нет — клиент уходит
		fmt.Printf("Клиент %d ушёл — нет свободных стульев.\n", id)
	}
}

func main() {
	rand.Seed(time.Now().UnixNano())

	go barber()

	// Запуск клиентов
	for i := 1; i <= NumClients; i++ {
		time.Sleep(time.Duration(rand.Intn(700)) * time.Millisecond)
		go client(i)
	}

	// Чтобы программа не завершалась мгновенно
	time.Sleep(10 * time.Second)
}
