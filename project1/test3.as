        lw      0       1       apple    
        lw      0       2       neg       
        add     3       1       2        
        sw      0       3       sky        
        beq     1       2       loop   
        jalr    0       1
        halt
apple   .fill   26
neg    .fill   -9
sky     .fill   100   
loop    noop             