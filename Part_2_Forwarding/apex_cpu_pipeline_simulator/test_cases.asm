---TC 1---
MOVC R0,#3
ADD R0,R0,R0
HALT 

---TC 2---
MOVC R0,#8
MOVC R1,#2
MOVC R2,#0
SUB R0,R0,R1
CMP R0,R2
BNZ #-8
BNP #20
SUBL R0,#2
NOP 
NOP 
NOP 
HALT 

---TC 3---
MOVC R2,#4
MOVC R4,#5
STORE R4,R2,#8
MOVC R3,#-2
ADDL R3,R3,#1
LOAD R5,R2,#8
HALT 


---TC 4---
MOVC R0,#10
MOVC R1,#20
MOVC R2,#4020
JALR R3,R2,#4
ADD R4,R0,R1
HALT 
ADDL R2,R2,#5
JUMP R3,#0


---TC 5---
MOVC R1,#10
MOVC R2,#4
ADDL R3,R1,#2
SUB R5,R3,R2
ADD R2,R2,R5
HALT 

---TC 6---
MOVC R0,#0
MOVC R1,#4
MOVC R2,#16
ADD R4,R0,R1
ADDL R0,R0,#4
CMP R0,R1
NOP 
BNZ #-16
ADDL R1,R1,#4
CMP R2,R1
BNZ #-28
HALT 


---TC 7---
MOVC R1,#10
MOVC R2,#2000
STOREP R1,R2,#8
LOADP R20,R2,#4
HALT 

---TC 8---
MOVC R1,#1000
MOVC R2,#2
MOVC R3,255
MOVC R4,#5
STOREP R4,R1,#4
ADDL R1,R1,#-4
LOADP R4,R1,#4
EX-OR R4,R4,R3
STOREP R4,R1,#4
SUBL R2,R2,#1
BP #-20
HALT 


------------------FORWARDING----------------------

---TC 1---