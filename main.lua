local rax_array = {
    "%al",  -- 8-bits register
    "%ax",  -- 16-bits register
    "%eax", -- 32-bits register
    "%rax", -- 64-bits register
}

local bytes = 4;
print(rax_array[mlog2(bytes)])
