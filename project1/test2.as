        lw 0 1 one
        lw 1 2 two
        lw 2 3 three 
start   add 1 1 2
        add 4 3 4
        beq 0 1 end
        beq 0 0 start
        noop
end     halt
one    .fill 6
two    .fill -2
three  .fill start