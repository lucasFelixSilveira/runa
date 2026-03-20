-- local rax_array = {
--     "%al",  -- 8-bits register
--     "%ax",  -- 16-bits register
--     "%eax", -- 32-bits register
--     "%rax", -- 64-bits register
-- }

-- local bits = 64
-- local bytes = bits // 8

-- local position = mlog2(bytes)
-- local register = rax_array[position]
-- print(register)

-- function fibonacci(x)
--     if x <= 1 then return x end
--     return fibonacci(x - 1) + fibonacci(x - 2)
-- end

-- fibonacci(12);

local z = 0
function test(ihatemylife)
    return "hello, " .. ihatemylife
end

local y = "world"
print(test(y) .. ihatemylife)
