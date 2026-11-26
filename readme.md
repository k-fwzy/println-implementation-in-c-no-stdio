# println/printlnf implementation in c (without stdio)

usage:
```c
    println("hello world"); // standard line print

    printlnf("9 + 10 = {}", 21); //formatted line print

    int* block = malloc(sizeof(int));
    *block = 21;
    printlnf("address of block {}, value: {}", block, *block);
    // output: address of block <addr>, value: 21

    printlnf("{:.3} {} whoops little clumsy :o", 3.141592); // messed up my values
    // output: 3.141 ((missing)) whoops little clumsy :o 

    printlnf("{{ message: {} }}", "i am the best programmer on earth");
    //output: { message: i am the best programmer on earth }

    printlnf("{ uh oh"); // messed up my formatting
    // output: println: unterminated '{'
```

run:
```bash
    # if your compiler supports c23
    gcc main.c && ./a.out
    clang main.c && ./a.out

    # otherwise add '-std=c23' or '-std=c2x'
    gcc main.c -std=c23 && ./a.out
    clang main.c -std=c23 && ./a.out

    gcc main.c -std=c2x && ./a.out
    clang main.c -std=c2x && ./a.out
```
