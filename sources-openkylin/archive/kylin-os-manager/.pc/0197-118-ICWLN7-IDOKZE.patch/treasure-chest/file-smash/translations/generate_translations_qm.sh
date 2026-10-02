#!/bin/bash

project_path=$(cd `dirname $0`; pwd)
cd $project_path/..

ts_file_list=(`ls translations/*.ts`)

for ts in "${ts_file_list[@]}"
do
    lrelease "${ts}"
done