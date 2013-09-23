#!/bin/bash

for i in *.som; do  
	if (grep primitive $i >/dev/null); then 
		echo -n '    "';
		echo $i |sed -e 's/.som/",/'; 
	fi; 
done
