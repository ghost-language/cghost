function fib(n)
    if n < 2 then
        return n
    else
        return fib(n - 1) + fib(n - 2)
    end
end

clock = os.clock
time_start = clock()
print(fib(35) == 9227465)
time_end = clock()

print("elapsed:")
print(time_end - time_start)