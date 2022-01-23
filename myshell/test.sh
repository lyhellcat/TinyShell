#!/bin/bash
echo `ls | grep "test"`
echo $[1+1]
echo $((1+1))
animal="cat"
echo $animal
echo ${animal}_food
X='Hello $LOGNAME'
echo $X
Y="Hello $LOGNAME"
echo $Y
echo -n "hello"
echo -e "hello\n"
echo "What is your name?"
read name
echo ¨Ce "Your name is $name." bob
echo 'Your name is $name.' $name