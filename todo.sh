#!/bin/bash

grep --exclude=todo.sh --color=auto -i -r TODO *
#find . | grep -v todo.sh  | xargs grep  TODO
