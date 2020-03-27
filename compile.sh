#!/bin/sh

cat stdlib.rkt $1 | node preprocessor/main.js | tee preprocessor_last_output.rkt | ./main > $1.s