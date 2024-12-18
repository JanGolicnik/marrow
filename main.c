#include "marrow.h"

int main() {
    println("xd");
    print("hey: ");
    print("hey\n");
    error("error");

    char* formatted = format("1 + 1 = %d", 1 + 1);
    debug("%s", formatted);

    return 0;
}
