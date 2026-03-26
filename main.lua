local rax_array = {
    "%al",  -- 1 Byte,  8 bits  // register
    "%ax",  -- 2 Bytes, 16 bits // register
    "%eax", -- 4 Bytes, 32 bits // register
    "%rax", -- 8 Bytes, 64 bits // register
}
local bytes = 8
local pos = mlog2(bytes)
print(rax_array)
print(rax_array[2])
