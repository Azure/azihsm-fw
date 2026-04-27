#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

top=`realpath .`

tag1=$1
tag2=$2

if [ $# -ne 2 ]; then
	printf "Usage: $0 [old_tag] [new_tag]\n"
	exit 1
fi

manticore_repo=$top/../
cerberus_core_repo=$top/../../../../../
cerberus_repo=$top/../../../
hsm_repo=$top/../cp/hsm/
qmgr_repo=$top/../fp/qmgr/
onefleet_hsp_repo=$top/../../
output_file=$top/"$tag2"_pr_list.md

printf "# Manticore firmware PR list from "*$tag1*" to "*$tag2*"" > $output_file

# Function to check if two tags exist in the repository
generate_pr_list() {
	local repo_path=$1

	# Navigate to the repository path
	cd "$repo_path" || { printf "Repository path not found\n"; exit 1; }

	# Check if both tags exist
	if git rev-parse "$tag1" "$tag2" >/dev/null 2>&1; then
		git log --pretty=format:%s $tag1..$tag2 | sed 's/^/| /; s/$/ |/' >> $output_file
		printf "|\n" >> $output_file
	else
		printf "One or both tags '$tag1' and '$tag2' do not exist in the repository.\n"
		rm $output_file
		cd -
		exit 1
	fi
}

# Get manticore repo features list
printf "\n## SP feature list" >> $output_file
printf "\n| Manticore repo |\n| --- |\n" >> $output_file
generate_pr_list $manticore_repo

# Get Cerberus Core repo features list
printf "\n| Cerberus Core repo |\n| --- |\n" >> $output_file
generate_pr_list $cerberus_core_repo

# Get Cerberus repo features list
printf "\n| Cerberus repo |\n| --- |\n" >> $output_file
generate_pr_list $cerberus_repo

# Get onefleet_hsp repo features list
printf "\n| Onefleet HSP repo |\n| --- |\n" >> $output_file
generate_pr_list $onefleet_hsp_repo

# Get HSM repo features list
printf "## CP feature list" >> $output_file
printf "\n| MCR HSM Repo |\n| --- |\n" >> $output_file
generate_pr_list $hsm_repo

# Get qmgr repo features list
printf "## FP Feature list" >> $output_file
printf "\n| Lion FP-QMGR repo |\n| --- |\n" >> $output_file
generate_pr_list $qmgr_repo
