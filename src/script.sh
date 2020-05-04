#!/bin/sh

sort $1 >temp1
sort $2 >temp2

if diff temp1 temp2 >/dev/null;
then echo "IGUAIS";
else echo "DIFFs";
fi

rm -f temp1 temp2