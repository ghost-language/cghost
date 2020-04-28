from __future__ import print_function

import time

def fib(n):
    if n < 2: return n
    return fib(n - 1) + fib(n - 2)

start = time.process_time()
print(fib(35) == 9227465)
end = time.process_time()

print("elapsed:")
print(str(end - start))