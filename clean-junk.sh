#!/bin/bash

for i in `find . -type f \( -iname \*.rej \
				-o -iname \*.orig \
				-o -iname \*.ko \
	rm -vf $i;
done;

