typedef struct{int a; int b; int c;}bad;

void boo(int* i){
  __asm volatile("nop"); // force decompilation

    bad* b = (bad*)i;
    b->c = 0xFFFFFFFF; // overflow
}

void main() {
  int i;

  boo(&i);
}
