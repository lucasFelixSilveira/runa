local ar = {
    "%al", -- 8-bits register
    "%ax", -- 16-bits register
    "%eax", -- 32-bits register
    "%rax", -- 64-bits register
}

print(ar[mlog2(32 // 8)])
