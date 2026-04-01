function fibonacci(n)
    local a = 0
    local b = 1

    for i = 1, n do
        print(a)
        local temp = a + b
        a = b
        b = temp
    end
end

fibonacci(80)
