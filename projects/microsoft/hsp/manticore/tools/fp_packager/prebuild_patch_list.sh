#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

function create_list_from_template() {
	local template=$1
	if [ -f $template ]; then
		local out_file=$(echo "$template" | sed "s|.template||g")
		cat $template | sed "s|TEMPLATE_PATH|$top|g" > $out_file
	else
		echo "$template does not exist."
		exit 0
	fi
}

top=`dirname "$(realpath "$BASH_SOURCE")"`
top=$(realpath "$top/../../")
echo "Updating list files to $top"

cp_list_template=./cp/hsm/target/thumbv7em-none-eabihf/firmware/cp.list.template
sprt_list_template=./sp/build-evb/sprt.list.template
sp1_list_template=./sp/build-evb/sp1.list.template
pcie_list_template=./pcie/build/pcie.list.template

create_list_from_template $cp_list_template
create_list_from_template $sprt_list_template
create_list_from_template $sp1_list_template
create_list_from_template $pcie_list_template
