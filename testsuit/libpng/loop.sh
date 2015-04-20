#!/bin/sh                                                                                                                                                              
                                                                                                                                                                       
trap  'exit 2 ' 15                                                                                                                                                     
                                                                                                                                                                       
count=0                                                                                                                                                                
                                                                                                                                                                       
while true; do                                                                                                                                                         
                                                                                                                                                                       
        cat /tmp/data0 | /tmp/fb -r                                                                                                                                    
        cat /tmp/data1 | /tmp/fb -r                                                                                                                                    
        count=$(($count+1))                                                                                                                                            
        echo "$(date '+%T') $count"                                                                                                                                    
                                                                                                                                                                       
done
