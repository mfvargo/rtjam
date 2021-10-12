#!/bin/bash
lame -r -s 48 - < ice_7891 2> /dev/null | ezstream -c ezstream.xml > /dev/null 2>&1
