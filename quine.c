#include <stdio.h>
#define pr putchar
int main() {
char qt = 34, nl = 10;
char* pro[] = {
"#include <stdio.h>",
"#define pr putchar",
"int main() {",
"char qt = 34, nl = 10;",
"char* pro[] = {",
"};",
"for (int i = 0; i < 5; i++) puts(pro[i]);",
"for (int i = 0; i < 12; i++) pr(qt), fputs(pro[i], stdout), pr(qt), pr(','), pr(nl);",
"puts(pro[5]);",
"for (int i = 6; i < 12; i++) puts(pro[i]);",
"return 0;",
"}",
};
for (int i = 0; i < 5; i++) puts(pro[i]);
for (int i = 0; i < 12; i++) pr(qt), fputs(pro[i], stdout), pr(qt), pr(','), pr(nl);
puts(pro[5]);
for (int i = 6; i < 12; i++) puts(pro[i]);
return 0;
}
