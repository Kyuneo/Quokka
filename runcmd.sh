#!/bin/bash

gcc src/quokka.c -o bin/quokka -ggdb3
if [[ $? != 0 ]]; then exit; fi
bin/quokka $1 $2 $3 $4
