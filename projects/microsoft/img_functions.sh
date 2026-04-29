# Copyright (c) Microsoft Corporation. All rights reserved.

empty_file() {
	echo -n "" > $1
}

output_binary_qword() {
	printf "%016x" $1 | tail -c 16 | sed -E 's/(..)(..)(..)(..)(..)(..)(..)(..)/\8\7\6\5\4\3\2\1/' | xxd -r -p >> $2
}

output_be_binary_qword() {
	printf "%016x" $1 | tail -c 16 | xxd -r -p >> $2
}

output_binary_word() {
	printf "%08x" $1 | tail -c 8 | sed -E 's/(..)(..)(..)(..)/\4\3\2\1/' | xxd -r -p >> $2
}

output_be_binary_word() {
	printf "%08x" $1 | tail -c 8 | xxd -r -p >> $2
}

output_binary_short() {
	printf "%04x" $1 | tail -c 4 | sed -E 's/(..)(..)/\2\1/' | xxd -r -p >> $2
}

output_be_binary_short() {
	printf "%04x" $1 | tail -c 4 | xxd -r -p >> $2
}

output_binary_byte() {
	printf "%02x" $1 | tail -c 2 | xxd -r -p >> $2
}

output_binary_array() {
	# Pad with an extra zero to align the array to 2 characters (1 byte).  If the array is already
	# aligned, the extra padding will get ignored.
	echo "${1}0" | xxd -r -p >> $2
}

output_ecc_public_key() {
	get_ecc_public_key $1 $3

	output_binary_array "$key_pub_x" "$2"
	output_binary_array "$key_pub_y" "$2"
}

generate_signature() {
	if [ -n "$4" ]; then
		algo=$4
	else
		algo="sha256"
	fi

	openssl dgst -$algo -sign $3 -out $2 $1
	if [ $? -ne 0 ]; then
		echo "Failed to sign $1."
		exit 1
	fi
}

generate_digest() {
	digest=`openssl dgst -$2 $1 | cut -d= -f 2`
	if [ $? -ne 0 ] || [ -z "$digest" ]; then
		echo "Failed to hash $1 with $2."
		exit 1
	fi

	if [ -n "$3" ]; then
		output_binary_array "$digest" "$3"
	fi
}

generate_sha256_hash() {
	openssl dgst -sha256 -binary -out $2 $1
	if [ $? -ne 0 ]; then
		echo "Failed to hash $1."
		exit 1
	fi
}

add_padding() {
	pad_length=`stat -c %s $1`
	total_length=$2
	let 'pad_length = total_length - pad_length'
	padding=''
	while [ $pad_length -gt 0 ]; do
		padding=${padding}${3}
		let 'pad_length--'
	done

	output_binary_array "$padding" "$1"
}

align_file() {
	file_len=`stat -c %s $1`
	alignment=$2
	let 'alignment = alignment - (file_len % alignment)'
	if [ $alignment -ne $2 ]; then
		let 'file_len = file_len + alignment'
		add_padding "$1" "$file_len" "$3"
	fi
}

pad_binary_array() {
	pad_length=`echo -n $1 | wc --bytes`
	total_length=$2
	let 'pad_length = total_length - pad_length'
	padded=$1
	while [ $pad_length -gt 0 ]; do
		padded=${3}${padded}
		let 'pad_length--'
	done
}

determine_key_type() {
	grep -q "PUBLIC" $1
	if [ $? -eq 0 ]; then
		# PEM public key file
		key_parse=`openssl asn1parse -in $1`
		if [ $? -ne 0 ]; then
			echo "$key_parse"
			exit 1
		fi

		echo $key_parse | grep -q 'rsaEncryption'
		if [ $? -eq 0 ]; then
			key_type="RSA"
		else
			key_type="ECC"
		fi
	else
		# PEM private key file
		grep -q RSA $1
		if [ $? -eq 0 ]; then
			key_type="RSA"
		else
			key_type="ECC"
		fi
	fi
}

get_public_key() {
	key_exp=`openssl rsa -text -noout < $1 2>/dev/null | grep publicExponent | awk '{print $2}'`
	if [ $? -ne 0 ] || [ -z "$key_exp" ]; then
		key_exp=`openssl rsa -text -noout -pubin < $1 | grep Exponent | awk '{print $2}'`
		if [ $? -ne 0 ] || [ -z "$key_exp" ]; then
			echo "Failed to get public exponent for key $1."
			exit 1
		fi

		pubin='-pubin'
	else
		pubin=
	fi

	if [ -z "$pubin" ]; then
		key_len=`openssl rsa -text -noout < $1 | grep Private-Key | sed -E 's/.*\((.*) bit.*/\1/'`
	else
		key_len=`openssl rsa -text -noout -pubin < $1 | grep Public-Key | sed -E 's/.*\((.*) bit.*/\1/'`
	fi
	if [ $? -ne 0 ] || [ -z "$key_len" ]; then
		echo "Failed to get key length for key $1."
		exit 1
	fi
	let 'key_len = key_len / 8'

	let 'lines = ((key_len + 1) + 14) / 15'
	key_mod=`openssl rsa -text -noout $pubin < $1 | tail --lines=+3 | head --lines=${lines} | tr -d '\n' | sed 's/://g' | sed 's/ //g' | tail --bytes=+3`
	if [ $? -ne 0 ] || [ -z "$key_mod" ]; then
		echo "Failed to get modulus for key $1."
		exit 1
	fi
}

check_ecc_curve() {
	key_curve=`openssl ec -text -noout -in $1 2>/dev/null | grep 'NIST CURVE' | awk '{print $3}'`
	if [ $? -ne 0 ] || [ -z "$key_curve" ]; then
		key_curve=`openssl ec -text -noout -pubin -in $1 2>/dev/null | grep 'NIST CURVE' | awk '{print $3}'`
		if [ $? -ne 0 ] || [ -z "$key_curve" ]; then
			echo "Failed to get curve name for key $1."
			exit 1
		fi

		pubin='-pubin'
	else
		pubin=
	fi

	if [ -n "$2" ] && [ "$key_curve" != "$2" ]; then
		echo "Wrong ECC curve for key $1.  Required $2, but uses $key_curve"
		exit 1
	fi
}

get_ecc_key_length() {
	if [ -z "$pubin" ]; then
		key_len=`openssl ec -text -noout -in $1 2>/dev/null | grep Private-Key | sed -E 's/.*\((.*) bit.*/\1/'`
	else
		key_len=`openssl ec -text -noout -pubin -in $1 2>/dev/null | grep Public-Key | sed -E 's/.*\((.*) bit.*/\1/'`
	fi
	if [ $? -ne 0 ] || [ -z "$key_len" ]; then
		echo "Failed to get key length for key $1."
		exit 1
	fi
	let 'key_len = (key_len + 7) / 8'
}

get_ecc_public_key() {
	check_ecc_curve $1 $2
	get_ecc_key_length $1

	let 'lines = (((key_len * 2) + 1) + 14) / 15'
	pub_line=`openssl ec -text -noout $pubin -in $1 2>/dev/null | grep -n "pub:" | cut -d: -f 1`
	let 'pub_line = pub_line + 1'
	key_pub=`openssl ec -text -noout $pubin -in $1 2>/dev/null | tail --lines=+${pub_line} | head --lines=${lines} | tr -d '\n' | sed 's/://g' | sed 's/ //g' | tail --bytes=+3`
	if [ $? -ne 0 ] || [ -z "$key_pub" ]; then
		echo "Failed to get public key for $1."
		exit 1
	fi

	let 'key_len_ascii = key_len * 2'
	key_pub_x=`echo -n ${key_pub} | head --bytes=${key_len_ascii}`
	key_pub_y=`echo -n ${key_pub} | tail --bytes=${key_len_ascii}`
}

get_ecc_der_public_key() {
	check_ecc_curve $1 $2
	get_ecc_key_length $1

	key_pub_der=`openssl ec -outform DER $pubin -pubout -in $1 2>/dev/null | xxd -p | tr -d '\n'`
	if [ $? -ne 0 ]; then
		echo "Failed to get DER public key for $1"
		echo "$key_pub_der"
		exit 1
	fi
}

get_ecc_der_public_key_max_length() {
	if [ -n "$1" ]; then
		check_ecc_curve $1 $2
		get_ecc_key_length $1
	fi

	case "$key_len" in
		32)
			algo_len=21
		;;

		48)
			algo_len=18
		;;

		66)
			algo_len=18
		;;

		*)
			echo "Unknown ECC key length $key_len"
			exit 1
		;;
	esac

	let 'der_len = (key_len * 2) + 4 + algo_len + 2'
	if [ $key_len -gt 61 ]; then
		let 'der_len = der_len + 2'
	fi
}

get_ecc_max_signature_length() {
	if [ -n "$1" ]; then
		check_ecc_curve $1 $2
		get_ecc_key_length $1
	fi

	let 'sig_len = ((key_len + 3) * 2) + 2'
	if [ $key_len -gt 61 ]; then
		let 'sig_len = sig_len + 1'
	fi
}

get_key_length() {
	determine_key_type $1
	if [ "$key_type" = "RSA" ]; then
		get_public_key $1
	else
		check_ecc_curve $1 $2
		get_ecc_key_length $1
	fi
}

get_max_signature_length() {
	determine_key_type $1
	if [ "$key_type" = "RSA" ]; then
		get_public_key $1
		sig_len=$key_len
	else
		get_ecc_max_signature_length $1
	fi
}

parse_ecc_signature() {
	sig_parsed=`openssl asn1parse -inform DER -in $1`
	if [ $? -ne 0 ] || [ -z "$sig_parsed" ]; then
		echo "Failed to parse ECC signature $1."
		exit 1
	fi

	sig_r=`echo -n "$sig_parsed" | tail --lines=+2 | head --lines=1 | tr -d '\n' | cut -d: -f 4`
	sig_s=`echo -n "$sig_parsed" | tail --lines=1 | tr -d '\n' | cut -d: -f 4`

	if [ -n "$2" ]; then
		sig_size=$2
		let 'sig_size = sig_size * 2'

		pad_binary_array "$sig_r" "$sig_size" "0"
		sig_r=$padded

		pad_binary_array "$sig_s" "$sig_size" "0"
		sig_s=$padded
	fi
}
