#include <iostream>
#include <cstdio>

extern "C" {
    double xeMain();
    double printd(double x){
       printf("%g\n", x);
       return 0.0;
    }
}

int main() {
    xeMain();
}
