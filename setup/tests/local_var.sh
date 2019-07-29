#!/bin/bash
HELLO=Hello 
function hello {
	local HELLO=World
	echo $HELLO
}
echo $HELLO
hello
echo $HELLO

n=8
echo "in sub script $n"
