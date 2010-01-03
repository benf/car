#!/bin/bash

grep --exclude=todo.sh --color=auto -r TODO *
#find . | grep -v todo.sh  | xargs grep  TODO
