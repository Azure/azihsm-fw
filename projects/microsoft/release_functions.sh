#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.


ROOT=`realpath $(dirname "$BASH_SOURCE")/../../..`

# Set release management properties.
if [ -z "$BRANCH_PREFIX" ]; then
	BRANCH_PREFIX='rel_branch_'
fi

if [ -z "$TAG_PREFIX" ]; then
	TAG_PREFIX='release_'
fi

if [ -z "$RELEASE_PREFIX" ]; then
	RELEASE_PREFIX='cerberus'
fi

if [ -z "$PRODUCT" ]; then
	PRODUCT='Cerberus'
fi

if [ -z "$VERSION_FILE" ]; then
	VERSION_FILE='src/version.h'
fi

if [ -z "$VERSION_REPO" ]; then
	VERSION_REPO='Cerberus'
fi

if [ -z "$MANIFEST_REPO" ]; then
	MANIFEST_REPO="$ROOT/.repo/manifests"
fi

if [ -z "$MANIFESTS" ]; then
	MANIFESTS='default.xml default-ssh.xml'
fi

if [ -z "$MANIFEST_MAIN_BRANCH" ]; then
	MANIFEST_MAIN_BRANCH='master'
fi

if [ -z "$MSFT_REPOS" ]; then
	MSFT_REPOS='cerberus-core Cerberus'
fi

if [ -z "$EXTERNAL_REPOS" ]; then
	EXTERNAL_REPOS='mbedtls'
fi

ALL_REPOS="$EXTERNAL_REPOS $MSFT_REPOS"

if [ -n "$SYSTEM_ACCESSTOKEN" ]; then
	RC_REPO='<INTERNAL_URL_REMOVED>
else
	RC_REPO='<INTERNAL_URL_REMOVED>
fi

debug_log() {
	# Usage: debug_log VAR_NAME "Description"
	local var_name="$1"
	local msg="$2"
	local var_value="${!var_name}"

	if [ "$ENABLE_LOGGING" != 1 ]; then
		return
	fi

	if [ -z "$msg" ]; then
		msg="Variable state"
	fi

	if [ -z "$var_value" ]; then
		echo "[LOG] $msg: '$var_name' is unset or empty."
	else
		echo "[LOG] $msg: '$var_name'='$var_value'"
	fi
}

parse_version_string() {
	if [ ! -e "$VERSION_FILE" ]; then
		echo "Can't find version file: $VERSION_FILE"
		exit 1
	fi

	if [ -n "$1" ]; then
		IFS='.' read -a version <<< "$1"
		if [ ${#version[@]} -ne 3 ]; then
			echo "Invalid version number specified: $1"
			exit 1
		fi
	else
		echo "No version number specified."
		exit 1
	fi

	major=${version[0]}
	minor=${version[1]}
	release=${version[2]}
	build=0
	BRANCH="${BRANCH_PREFIX}$major.$minor.$release"
}

get_build_version() {
	major=`grep -m 1 FW_VERSION_MAJOR $VERSION_FILE | awk '{print $3}' | tr -d '\r\n'`
	minor=`grep -m 1 FW_VERSION_MINOR $VERSION_FILE | awk '{print $3}' | tr -d '\r\n'`
	release=`grep -m 1 FW_VERSION_RELEASE $VERSION_FILE | awk '{print $3}' | tr -d '\r\n'`
	build=`grep -m 1 FW_VERSION_BUILD $VERSION_FILE | awk '{print $3}' | tr -d '\r\n'`

	debug_log "major" "Major version number"
	debug_log "minor" "Minor version number"
	debug_log "release" "Release version number"
	debug_log "build" "Build version number"

	# Check that the branch name matches the version configuration
	check=`echo $BRANCH | awk -F '_' '{print $NF}'`

	IFS='.' read -a version <<< "$check"
	if [ ${#version[@]} -ne 3 ]; then
		echo "Invalid branch version number: $BRANCH"
		exit 1
	fi

	if [ "x$major" != "x${version[0]}" ]; then
		echo "Misconfigured major version number: $major"
		exit 1
	fi

	if [ "x$minor" != "x${version[1]}" ]; then
		echo "Misconfigured minor version number: $minor"
		exit 1
	fi

	if [ "x$release" != "x${version[2]}" ]; then
		echo "Misconfigured release version number: $release"
		exit 1
	fi

	export RELEASE_VER="$major.$minor.$release.$build"
	export ARTIFACT_VERSION_SHORT="$minor.$release.$build"
	export ARTIFACT_VERSION_FULL="$major.$minor.$release-$build"
	debug_log "RELEASE_VER" "Release Version"
	debug_log "ARTIFACT_VERSION_SHORT" "artifact version short"
	debug_log "ARTIFACT_VERSION_FULL" "artifact version full"
}

set_branch_version() {
	python << EOF
import re

ver_file = open ("$VERSION_FILE", "r")
ver_data = ver_file.read ()
ver_file.close ()

ver_data = re.sub ("(#define\s+FW_VERSION_MAJOR\s+)\d+", "\g<1>$major", ver_data)
ver_data = re.sub ("(#define\s+FW_VERSION_MINOR\s+)\d+", "\g<1>$minor", ver_data)
ver_data = re.sub ("(#define\s+FW_VERSION_RELEASE\s+)\d+", "\g<1>$release", ver_data)
ver_data = re.sub ("(#define\s+FW_VERSION_BUILD\s+)\d+", "\g<1>$build", ver_data)
ver_data = re.sub ("(#define\s+FW_VERSION_IS_RELEASE\s+)\d+", "\g<1>1", ver_data)

ver_file = open ("$VERSION_FILE", "w")
ver_file.write (ver_data)
ver_file.close ()
EOF

	git_add $VERSION_FILE
	git_commit "Set version number for ${PRODUCT} FW v$major.$minor.$release release branch."
}

update_version_number() {
	python << EOF
import re

ver_file = open ("$VERSION_FILE", "r")
ver_data = ver_file.read ()
ver_file.close ()

next_build = $build + 1
ver_data = re.sub ("(#define\s+FW_VERSION_BUILD\s+)\d+", "\g<1>{0}".format (next_build), ver_data)

ver_file = open ("$VERSION_FILE", "w")
ver_file.write (ver_data)
ver_file.close ()
EOF

	get_repo_information $VERSION_REPO
	debug_log "VERSION_FILE" "git add"
	git_add $VERSION_FILE
	git_commit "Bump version number on ${PRODUCT} v$major.$minor.$release release branch."
	git push ${repo_info[0]} $BRANCH
	if [ $? -ne 0 ]; then
		echo "Failed to push updated version file: $BRANCH"
		exit 1
	fi
}

get_current_release_branch() {
	get_repo_branch
	BRANCH=`echo $BRANCH | grep "^${BRANCH_PREFIX}"`
	if [ -z "$BRANCH" ]; then
		echo "Not currently on a release branch."
		exit 1
	fi
}

get_repo_branch() {
	REPO_BRANCH=`git -C $MANIFEST_REPO rev-parse --abbrev-ref HEAD`
	if [ $? -ne 0 ] || [ -z "$REPO_BRANCH" ]; then
		echo "Current branch is unknown.",
		exit 1
	fi
	debug_log "REPO_BRANCH" "Current repository branch"

	origin=($(git -C $MANIFEST_REPO remote show origin | grep "$REPO_BRANCH merges with"))
	debug_log "origin" "origin"
	BRANCH=${origin[${#origin[@]}-1]}
}

check_repo_branch() {
	get_repo_branch
	if [ "$BRANCH" != "$1" ]; then
		echo "Only run from repos initialized with branch $1."
		exit 1
	fi
}

repo_sync() {
	repo sync
	if [ $? -ne 0 ]; then
		echo "Failed to synchronize working copy."
		exit 1
	fi
}

switch_to_branch() {
	debug_log "MSFT_REPOS" "Switching to branch"
	repo start $1 $MSFT_REPOS
	if [ $? -ne 0 ]; then
		echo "Failed to switch to branch: $1."
		exit 1
	fi
}

switch_to_main_branch() {
	# Some repos use 'master' and others use 'main', this difference needs to be handled.
	master_branch=
	main_branch=
	for repo in $MSFT_REPOS; do
		if [ "$repo" = "cerberus-core" ] || [ "$repo" = "Cerberus" ]; then
			master_branch="$master_branch $repo"
		else
			main_branch="$main_branch $repo"
		fi
	done

	repo start master $master_branch
	if [ $? -ne 0 ]; then
		echo "Failed to switch to branch: master."
		exit 1
	fi

	repo start main $main_branch
	if [ $? -ne 0 ]; then
		echo "Failed to switch to branch: master."
		exit 1
	fi
}

push_release_branch() {
	debug_log "BRANCH" "Pushing release branch to remote repos"
	repo forall $MSFT_REPOS -c "git push -u \$REPO_REMOTE $BRANCH"
	ret=$?
	if [ $ret -ne 0 ]; then
		echo "Failed to push release branches to remote return $ret."
		exit 1
	fi
}

branch_repo_manifest() {
	git -C $MANIFEST_REPO checkout -b $BRANCH
	if [ $? -ne 0 ]; then
		echo "Failed to create repo manifest branch: $BRANCH"
		exit 1
	fi
}

get_repo_revisions() {
	if [ -n "$1" ]; then
		check="$1"
	else
		check="$ALL_REPOS"
	fi

	revisions=`repo forall $check -c 'echo "$REPO_REMOTE,$REPO_PROJECT,$REPO_PATH,$REPO_LREV,$REPO_RREV"'`
	if [ $? -ne 0 ]; then
		echo "Failed to get current git revisions for all repositories."
		exit 1
	fi
}

get_repo_information() {
	get_repo_revisions $1
	IFS=',' read -a repo_info <<< "$revisions"
}

update_manifest() {
	get_repo_revisions
	for r in $revisions; do
		IFS=',' read -a update <<< "$r"
		for m in $MANIFESTS; do
			echo -n "$MSFT_REPOS" | grep -q "${update[1]}"
			if [ $? -eq 0 ] && [ -z "$1" ]; then
				sed -i "/name=\"${update[1]}\"/ s/revision=\"${update[4]}\"/revision=\"$BRANCH\"/" $MANIFEST_REPO/$m
			else
				sed -i "/name=\"${update[1]}\"/ s/revision=\"${update[4]}\"/revision=\"${update[3]}\"/" $MANIFEST_REPO/$m
			fi
		done
	done
}

commit_repo_manifest() {
	git -C $MANIFEST_REPO status | grep -q ".xml"
	if [ $? -eq 0 ]; then
		debug_log "MANIFESTS" "Committing manifest files"
		git -C $MANIFEST_REPO add $MANIFESTS
		if [ $? -ne 0 ]; then
			echo "Failed to add manifest files: $MANIFESTS"
			exit 1
		fi

		if [ -z "$2" ]; then
			msg="Update repo manifest on ${PRODUCT} v$major.$minor.$release release branch."
		else
			msg="Update repo manifest for ${PRODUCT} FW release v$RELEASE_VER."
		fi
		git -C $MANIFEST_REPO commit -m "$msg"
		if [ $? -ne 0 ]; then
			echo "Failed to commit manifest files: $MANIFESTS"
			exit 1
		fi

		if [ -z "$REPO_BRANCH" ]; then
			git -C $MANIFEST_REPO push -u origin $1
		else
			git -C $MANIFEST_REPO push origin $REPO_BRANCH:$1
		fi
		if [ $? -ne 0 ]; then
			echo "Failed to push updated manifest files: $MANIFESTS"
			exit 1
		fi
	else
		echo "Manifest for $RELEASE_VER has not changed."
	fi
}

get_release_image() {
	out_dir=`dirname $IMAGE`
	RELEASE_IMG="$out_dir/${RELEASE_PREFIX}_v${RELEASE_VER}${VERSION_EXT}.bin"
	cp $IMAGE $RELEASE_IMG

	if [ -n "$CFM" ]; then
		out_dir=`dirname $CFM`
		RELEASE_IMG_CFM="$out_dir/CFM_v${RELEASE_VER}${VERSION_EXT}.xml"
		cp $CFM $RELEASE_IMG_CFM
	fi

	if [ -n "$CHKSUM" ]; then
		out_dir=`dirname $CHKSUM`
		RELEASE_CHKSUM="$out_dir/checksum_v${RELEASE_VER}${VERSION_EXT}.txt"
		cp $CHKSUM $RELEASE_CHKSUM
	fi

	if [ -n "$AUTH" ]; then
		out_dir=`dirname $AUTH`
		auth_name=`basename $AUTH .bin`
		RELEASE_AUTH="$out_dir/${auth_name}_v${RELEASE_VER}${VERSION_EXT}.bin"
		cp $AUTH $RELEASE_AUTH
	fi
}

store_release_image() {
	get_release_image

	# Store the release candidate image in the RC repository.
	rel_dir=v$major.$minor.$release.x
	rc_dir=`mktemp -d --tmpdir=.`

	if [ -n "$SYSTEM_ACCESSTOKEN" ]; then
		git -c http.${RC_REPO}.extraheader="AUTHORIZATION: bearer $SYSTEM_ACCESSTOKEN" clone $RC_REPO $rc_dir
	else
		git clone $RC_REPO $rc_dir
	fi
	if [ $? -ne 0 ]; then
		echo "Failed to clone RC repo: $RC_REPO"
		rm -rf $rc_dir
		exit 1
	fi

	if [ -n "$SYSTEM_ACCESSTOKEN" ]; then
		git -C $rc_dir config http.${RC_REPO}.extraheader "AUTHORIZATION: bearer $SYSTEM_ACCESSTOKEN"
		if [ $? -ne 0 ]; then
			echo "Failed to configure repo authorization."
			exit 1
		fi
	fi

	mkdir -p $rc_dir/$rel_dir
	cp $RELEASE_IMG $rc_dir/$rel_dir
	if [ -n "$RELEASE_IMG_CFM" ]; then
		cp $RELEASE_IMG_CFM $rc_dir/$rel_dir
	fi
	if [ -n "$RELEASE_CHKSUM" ]; then
		cp $RELEASE_CHKSUM $rc_dir/$rel_dir
	fi

	git -C $rc_dir add $rel_dir
	if [ $? -ne 0 ]; then
		echo "Failed to add RC image: $rel_dir/$RELEASE_IMG"
		rm -rf $rc_dir
		exit 1
	fi

	git -C $rc_dir commit -m "RC build for ${PRODUCT} FW release v$RELEASE_VER"
	if [ $? -ne 0 ]; then
		echo "Failed to commit RC image."
		rm -rf $rc_dir
		exit 1
	fi

	git -C $rc_dir push origin master
	if [ $? -ne 0 ]; then
		echo "Failed to push RC image."
		rm -rf $rc_dir
		exit 1
	fi

	rm -rf $rc_dir
}

tag_release_build() {
	TAG="$TAG_PREFIX$RELEASE_VER"
	debug_log "TAG" "TAG for release build"
	repo forall $MSFT_REPOS -c "git tag -a $TAG -m '${PRODUCT} FW release v$RELEASE_VER'"
	if [ $? -ne 0 ]; then
		echo "Failed to tag release build $RELEASE_VER."
		exit 1
	fi

	repo forall $MSFT_REPOS -c "git push \$REPO_REMOTE $TAG"
	if [ $? -ne 0 ]; then
		echo "Failed to push build tags to remote."
		exit 1
	fi

	git -C $MANIFEST_REPO tag -a $TAG -m "${PRODUCT} FW release v$RELEASE_VER"
	if [ $? -ne 0 ]; then
		echo "Failed to tag release $RELEASE_VER manifest.".
		exit 1
	fi

	git -C $MANIFEST_REPO push origin $TAG
	if [ $? -ne 0 ]; then
		echo "Failed to push manifest tag to remote."
		exit 1
	fi
}

git_add() {
	git add $1
	if [ $? -ne 0 ]; then
		echo "Failed to add $1 to git."
		exit 1
	fi
}

git_commit() {
	git commit -m "$1"
	if [ $? -ne 0 ]; then
		echo "Failed git commit: $1"
		exit 1
	fi
}

make_release_branch() {
	# Make sure the local copy is based on master and the script is run from the right directory
	debug_log "MANIFEST_REPO" "Manifest repository"
	check_repo_branch ${MANIFEST_MAIN_BRANCH}
	switch_to_main_branch
	parse_version_string $1

	# Create the branch and set the version information.
	branch_repo_manifest
	switch_to_branch $BRANCH
	set_branch_version

	# Push the new branch to the remote.
	push_release_branch

	# Update the repo manifest to reference the branch.
	TRACK="-u"
	REPO_BRANCH=""
	update_manifest
	commit_repo_manifest $BRANCH
}

create_or_switch_to_release_branch() {
	local branch_name="$BRANCH_PREFIX$1"
	if git -C $MANIFEST_REPO branch -a | grep $branch_name; then
		debug_log "branch_name" "Switching to existing branch"
		git -C $MANIFEST_REPO switch $branch_name
		switch_to_branch $branch_name
		repo_sync
		if [ $? -ne 0 ]; then
			echo "Failed to pull the release branch $branch_name"
			exit 1
		fi

		return 0
	else
		debug_log "branch_name" "Creating new release branch"
		make_release_branch $1
	fi
}


prepare_for_release_build() {
	get_current_release_branch

	# Make sure we are up-to-date and determine the version of the build.
	repo_sync
	switch_to_branch $BRANCH
	get_build_version
	debug_log "RELEASE_VER" "prepare_for_release_build Release version"

	# Update the manifest to bind to specific commit IDs
	update_manifest 1
	commit_repo_manifest $BRANCH 1
}

save_release_build() {
	# Tag the release.
	tag_release_build

	# Save the incremented build version and revert the manifest back to the branch HEAD
	update_version_number
	update_manifest
	debug_log "BRANCH" "Committing repo manifest"
	commit_repo_manifest $BRANCH
}

build_release() {
	prepare_for_release_build

	# Build the image.
	#
	# The path to the image that was build must be stored in the variable IMAGE after execution.
	# Optionally, the CHKSUM variable can be populated with a checksum file for the IMAGE.
	build_release_image

	store_release_image
	save_release_build
	echo $RELEASE_IMG
}

build_release_no_commit_rc() {
	prepare_for_release_build

	# Build the image.
	#
	# The path to the image that was build must be stored in the variable IMAGE after execution.
	# Optionally, the CHKSUM variable can be populated with a checksum file for the IMAGE.
	build_release_image

	get_release_image
	save_release_build
	echo $RELEASE_IMG
}
