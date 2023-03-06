#!/bin/bash

rm -rf gen mydir err.txt myfile test

cat >test.c <<'EOF'
#include <stdio.h>
int main(void) {
	printf("this is test\n");
	return 0;
}
EOF

echo this is moveme > moveme
echo this is copyme > copyme
echo this is readme > readme

cat >bad.c <<'EOF'
int main(void) {
	foo();
	bad;
	return 0;
}
EOF
