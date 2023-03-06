.text
 .globl go
go:
 pushq %rbx
 pushq %rbp
 pushq %r12
 pushq %r13
 pushq %r14
 pushq %r15
 call _main
 popq %r15
 popq %r14
 popq %r13
 popq %r12
 popq %rbp
 popq %rbx
 retq
_main:
 subq $160, %rsp
 _entry_point0_main:
 jmp _entry_point1_main
 _entry_point1_main:
 movq $1, %rdi
 movq %rdi, 112(%rsp)
 movq $1, %rdi
 movq %rdi, 120(%rsp)
 movq $1, %rdi
 movq %rdi, 32(%rsp)
 call input
 movq %rax, %rdi
 movq %rdi, 32(%rsp)
 jmp _body_main
 _body_main:
 movq $1, %rdi
 movq %rdi, 72(%rsp)
 call input
 movq %rax, %rdi
 movq %rdi, 72(%rsp)
 movq 112(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 8(%rsp)
 movq 8(%rsp), %rdi
 sarq $1, %rdi
 movq %rdi, 8(%rsp)
 movq 8(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 24(%rsp)
 movq 72(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 16(%rsp)
 movq 16(%rsp), %rdi
 sarq $1, %rdi
 movq %rdi, 16(%rsp)
 movq 16(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 56(%rsp)
 movq 24(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 64(%rsp)
 movq 56(%rsp), %rsi
 movq 64(%rsp), %rdi
 addq %rsi, %rdi
 movq %rdi, 64(%rsp)
 movq 64(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 112(%rsp)
 movq 112(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 64(%rsp)
 movq 64(%rsp), %rdi
 salq $1, %rdi
 movq %rdi, 64(%rsp)
 movq 64(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 112(%rsp)
 movq 112(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 64(%rsp)
 movq 64(%rsp), %rdi
 addq $1, %rdi
 movq %rdi, 64(%rsp)
 movq 64(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 112(%rsp)
 movq 120(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 80(%rsp)
 movq 80(%rsp), %rdi
 sarq $1, %rdi
 movq %rdi, 80(%rsp)
 movq 80(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 0(%rsp)
 movq 0(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 40(%rsp)
 movq 40(%rsp), %rdi
 addq $1, %rdi
 movq %rdi, 40(%rsp)
 movq 40(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 120(%rsp)
 movq 120(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 40(%rsp)
 movq 40(%rsp), %rdi
 salq $1, %rdi
 movq %rdi, 40(%rsp)
 movq 40(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 120(%rsp)
 movq 120(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 40(%rsp)
 movq 40(%rsp), %rdi
 addq $1, %rdi
 movq %rdi, 40(%rsp)
 movq 40(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 120(%rsp)
 jmp _new_label0_main
 _new_label0_main:
 movq $1, %rdi
 movq %rdi, 96(%rsp)
 movq 120(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 104(%rsp)
 movq 104(%rsp), %rdi
 sarq $1, %rdi
 movq %rdi, 104(%rsp)
 movq 104(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 136(%rsp)
 movq 32(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 152(%rsp)
 movq 152(%rsp), %rdi
 sarq $1, %rdi
 movq %rdi, 152(%rsp)
 movq 152(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 88(%rsp)
 movq 88(%rsp), %rsi
 movq 136(%rsp), %rdi
 cmpq %rsi, %rdi
 setl %dil
 movzbq %dil, %rdi
 movq %rdi, 96(%rsp)
 movq 96(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 128(%rsp)
 movq 128(%rsp), %rdi
 salq $1, %rdi
 movq %rdi, 128(%rsp)
 movq 128(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 96(%rsp)
 movq 96(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 128(%rsp)
 movq 128(%rsp), %rdi
 addq $1, %rdi
 movq %rdi, 128(%rsp)
 movq 128(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 96(%rsp)
 movq 96(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 144(%rsp)
 movq 144(%rsp), %rdi
 sarq $1, %rdi
 movq %rdi, 144(%rsp)
 movq 144(%rsp), %rdi
 movq %rdi, %rdi
 movq %rdi, 48(%rsp)
 movq 48(%rsp), %rdi
 cmpq $1, %rdi
 je _body_main
 jmp _exit_main
 _exit_main:
 movq 112(%rsp), %rdi
 movq %rdi, %rdi
 call print
 addq $160, %rsp
 retq
