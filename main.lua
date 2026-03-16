local rax_array = {
    "%al",  -- 8-bits register
    "%ax",  -- 16-bits register
    "%eax", -- 32-bits register
    "%rax", -- 64-bits register
}

local bits = 64
local bytes = bits // 8

local position = mlog2(bytes)
local register = rax_array[position]
print(register)
