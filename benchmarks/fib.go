package main

import (
	"fmt"
	"time"
)

func main() {
	start := time.Now()

	fib(35)

	end := time.Now()
	elapsed := end.Sub(start)

	fmt.Printf("elapsed: %f", elapsed.Seconds())
}

func fib(n int) int {
	if n < 2 {
		return n
	}

	return fib(n-2) + fib(n-1)
}
