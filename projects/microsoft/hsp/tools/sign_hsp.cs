using System;
using System.Collections.Generic;
using System.IO;
using System.Security.Cryptography;
using System.Runtime.InteropServices;
using SpSign;

const uint NUL_MAGIC				= 0x4e554c4c;
const uint OTM_MAGIC				= 0x6f776e74;
const uint OTM_MAGIC_DUAL_ROOT		= 0x6f746d32;
const uint TGM_MAGIC				= 0x746d616e;
const uint RKM_MAGIC				= 0x4d414e54;
const uint RKM_MAGIC_DUAL_ROOT		= 0x524b4d32;
const uint SP1_MAGIC				= 0x53504657;
const uint FKM_MAGIC				= 0x46574b4d;
const uint FKM_MAGIC_V2				= 0x4d4b5746;

const int SP_MSG_384_SIZE						= SpUtil.SpMsg384Size;
const int SP_ECDSA_P384_SIGNATURE_SIZE			= SpUtil.Ecc384SignatureSize;
const int SP_ECDSA_P384_PUBLIC_SIZE				= SpUtil.Ecc384PubKeySize;
const int DER_ECDSA_P384_SIGNATURE_MAX_SIZE		= 8 + (SP_MSG_384_SIZE * 2);
const int DER_ECDSA_P384_PUBLIC_MAX_SIZE		= 24 + (SP_MSG_384_SIZE * 2);

const int NUL_HDR_SIZE					= 4;
const int TGM_HDR_SIZE					= 300;
const int OTM_HDR_SIZE					= 344;
const int OTM_HDR_SIZE_DUAL_ROOT		= 632;
const int RKM_HDR_SIZE					= 448;
const int RKM_HDR_SIZE_DUAL_ROOT		= 640;
const int SP1_HDR_SIZE					= 284;
const int FKM_HDR_SIZE					= 1472;
const int FKM_HDR_SIZE_DUAL_ROOT		= 2684;

const int IO_BUF_SIZE		= 3072;

const long OPERATION_NONE								= (long) 0;
const long OPERATION_VERIFY_ROOT_KEY_MANIFEST			= (long) 1 << 0;
const long OPERATION_VERIFY_ROOT_KEY					= (long) 1 << 1;
const long OPERATION_VERIFY_SIGNING_KEY					= (long) 1 << 2;
const long OPERATION_VERIFY_SECONDARY_KEY				= (long) 1 << 3;
const long OPERATION_SIGN_ROOT_KEY_MANIFEST				= (long) 1 << 4;
const long OPERATION_MANIFEST_SET_ROOT_KEY				= (long) 1 << 5;
const long OPERATION_MANIFEST_SET_SIGNING_KEY			= (long) 1 << 6;
const long OPERATION_MANIFEST_SET_SECONDARY_KEY			= (long) 1 << 7;
const long OPERATION_VERIFY_TRANSFER					= (long) 1 << 8;
const long OPERATION_VERIFY_OLD_OWNER_KEY				= (long) 1 << 9;
const long OPERATION_VERIFY_NEW_OWNER_KEY				= (long) 1 << 10;
const long OPERATION_SIGN_TRANSFER_MANIFEST				= (long) 1 << 11;
const long OPERATION_TRANSFER_SET_NEW_OWNER				= (long) 1 << 12;
const long OPERATION_VERIFY_TENANCY						= (long) 1 << 13;
const long OPERATION_VERIFY_TENANT_KEY					= (long) 1 << 14;
const long OPERATION_SIGN_TENANCY						= (long) 1 << 15;
const long OPERATION_TENANT_SET_KEY						= (long) 1 << 16;
const long OPERATION_VERIFY_FIRMWARE					= (long) 1 << 17;
const long OPERATION_SIGN_FIRMWARE_PRIMARY				= (long) 1 << 18;
const long OPERATION_SIGN_FIRMWARE_SECONDARY			= (long) 1 << 19;
const long OPERATION_VERIFY_FW_KEYS_PRIMARY				= (long) 1 << 20;
const long OPERATION_VERIFY_FW_KEYS_SECONDARY			= (long) 1 << 21;
const long OPERATION_SIGN_FW_KEYS_PRIMARY				= (long) 1 << 22;
const long OPERATION_SIGN_FW_KEYS_SECONDARY				= (long) 1 << 23;
const long OPERATION_VERIFY_ROOT_KEY_MANIFEST_SECONDARY	= (long) 1 << 24;
const long OPERATION_VERIFY_ROOT_KEY_SECONDARY			= (long) 1 << 25;
const long OPERATION_SIGN_ROOT_KEY_MANIFEST_SECONDARY	= (long) 1 << 26;
const long OPERATION_MANIFEST_SET_ROOT_KEY_SECONDARY	= (long) 1 << 27;
const long OPERATION_VERIFY_TRANSFER_SECONDARY			= (long) 1 << 28;
const long OPERATION_VERIFY_OLD_OWNER_KEY_SECONDARY		= (long) 1 << 29;
const long OPERATION_VERIFY_NEW_OWNER_KEY_SECONDARY		= (long) 1 << 30;
const long OPERATION_SIGN_TRANSFER_MANIFEST_SECONDARY	= (long) 1 << 31;
const long OPERATION_TRANSFER_SET_NEW_OWNER_SECONDARY	= (long) 1 << 32;
const long OPERATION_MASK							= (OPERATION_TRANSFER_SET_NEW_OWNER_SECONDARY << 1) - 1;

const long MANIFEST_OPERATION_VERIFY		= OPERATION_VERIFY_ROOT_KEY_MANIFEST |
	OPERATION_VERIFY_ROOT_KEY | OPERATION_VERIFY_SIGNING_KEY | OPERATION_VERIFY_SECONDARY_KEY |
	OPERATION_VERIFY_ROOT_KEY_MANIFEST_SECONDARY | OPERATION_VERIFY_ROOT_KEY_SECONDARY;
const long MANIFEST_OPERATION_UPDATE		= OPERATION_SIGN_ROOT_KEY_MANIFEST |
	OPERATION_MANIFEST_SET_ROOT_KEY | OPERATION_MANIFEST_SET_SIGNING_KEY |
	OPERATION_MANIFEST_SET_SECONDARY_KEY | OPERATION_SIGN_ROOT_KEY_MANIFEST_SECONDARY |
	OPERATION_MANIFEST_SET_ROOT_KEY_SECONDARY;
const long MANIFEST_OPERATION_MASK		= MANIFEST_OPERATION_VERIFY | MANIFEST_OPERATION_UPDATE;

const long TRANSFER_OPERATION_VERIFY		= OPERATION_VERIFY_TRANSFER |
	OPERATION_VERIFY_OLD_OWNER_KEY | OPERATION_VERIFY_NEW_OWNER_KEY | OPERATION_VERIFY_TRANSFER_SECONDARY |
	OPERATION_VERIFY_OLD_OWNER_KEY_SECONDARY | OPERATION_VERIFY_NEW_OWNER_KEY_SECONDARY;
const long TRANSFER_OPERATION_UPDATE		= OPERATION_SIGN_TRANSFER_MANIFEST |
	OPERATION_TRANSFER_SET_NEW_OWNER | OPERATION_SIGN_TRANSFER_MANIFEST_SECONDARY | OPERATION_TRANSFER_SET_NEW_OWNER_SECONDARY;
const long TRANSFER_OPERATION_MASK		= TRANSFER_OPERATION_VERIFY | TRANSFER_OPERATION_UPDATE;

const long TENANT_OPERATION_VERIFY		= OPERATION_VERIFY_TENANCY | OPERATION_VERIFY_TENANT_KEY;
const long TENANT_OPERATION_UPDATE		= OPERATION_SIGN_TENANCY | OPERATION_TENANT_SET_KEY;
const long TENANT_OPERATION_MASK			= TENANT_OPERATION_VERIFY | TENANT_OPERATION_UPDATE;

const long FIRMWARE_OPERATION_VERIFY		= OPERATION_VERIFY_FIRMWARE;
const long FIRMWARE_OPERATION_UPDATE		= OPERATION_SIGN_FIRMWARE_PRIMARY |
	OPERATION_SIGN_FIRMWARE_SECONDARY;
const long FIRMWARE_OPERATION_MASK		= FIRMWARE_OPERATION_VERIFY | FIRMWARE_OPERATION_UPDATE;

const long FW_KEYS_OPERATION_VERIFY		= OPERATION_VERIFY_FW_KEYS_PRIMARY |
	OPERATION_VERIFY_FW_KEYS_SECONDARY;
const long FW_KEYS_OPERATION_UPDATE		= OPERATION_SIGN_FW_KEYS_PRIMARY |
	OPERATION_SIGN_FW_KEYS_SECONDARY;
const long FW_KEYS_OPERATION_MASK		= FW_KEYS_OPERATION_VERIFY | FW_KEYS_OPERATION_UPDATE;

const long SP1_OPERATION_MASK = MANIFEST_OPERATION_MASK | TRANSFER_OPERATION_MASK |
	FIRMWARE_OPERATION_MASK;

const long OPERATION_UPDATE_MASK = MANIFEST_OPERATION_UPDATE | TRANSFER_OPERATION_UPDATE |
	TENANT_OPERATION_UPDATE | FIRMWARE_OPERATION_UPDATE | FW_KEYS_OPERATION_UPDATE;


// spcryptotypes.h

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1,
	Size = SP_MSG_384_SIZE)]
struct SP_MSG_384 {
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = SP_MSG_384_SIZE)]
	public byte[] data;
}

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1,
	Size = SP_ECDSA_P384_SIGNATURE_SIZE)]
struct SP_ECDSA_P384_SIGNATURE {
	public SP_MSG_384 R;
	public SP_MSG_384 S;
}

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1,
	Size = SP_ECDSA_P384_PUBLIC_SIZE)]
struct SP_ECDSA_P384_PUBLIC {
	public SP_MSG_384 X;
	public SP_MSG_384 Y;
}

// ecc_der_util.h

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1,
	Size = DER_ECDSA_P384_SIGNATURE_MAX_SIZE)]
struct DER_ECDSA_P384_SIGNATURE {
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = DER_ECDSA_P384_SIGNATURE_MAX_SIZE)]
	public byte[] data;
}

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1,
	Size = DER_ECDSA_P384_PUBLIC_MAX_SIZE)]
struct DER_ECDSA_P384_PUBLIC {
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = DER_ECDSA_P384_PUBLIC_MAX_SIZE)]
	public byte[] data;
}

// key_manifest_hsp_rom.h

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
struct key_manifest_hsp_rom_grant_header_signed {
	public byte type;
	[MarshalAs (UnmanagedType.ByValArray, SizeConst = 3)]
	private byte[] reserved;
	public uint length;
	public SP_MSG_384 digest;
}

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
struct key_manifest_hsp_rom_grant_data {
	public SP_MSG_384 grant_token;
	public SP_ECDSA_P384_PUBLIC tenant_key;
}

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1, Size = TGM_HDR_SIZE)]
struct key_manifest_hsp_rom_grant {
	public uint marker;
	public SP_ECDSA_P384_SIGNATURE signature;
	public key_manifest_hsp_rom_grant_header_signed header_signed;
	public key_manifest_hsp_rom_grant_data data;
}

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
struct key_manifest_hsp_rom_ownership_header_signed {
	public uint length;
	public SP_MSG_384 digest;
}

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
struct key_manifest_hsp_rom_ownership_data {
	public SP_ECDSA_P384_PUBLIC new_owner_key;
}

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1, Size = OTM_HDR_SIZE)]
struct key_manifest_hsp_rom_ownership {
	public uint marker;
	public SP_ECDSA_P384_PUBLIC owner_key;
	public SP_ECDSA_P384_SIGNATURE signature;
	public key_manifest_hsp_rom_ownership_header_signed header_signed;
	public key_manifest_hsp_rom_ownership_data data;
}

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
struct key_manifest_hsp_rom_keys_header_signed {
	public byte type;
	public byte valid_keys;
	public ushort reserved;
	public uint svn;
	public uint length;
	public SP_MSG_384 digest;
}

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
struct key_manifest_hsp_rom_keys_data {
	public SP_ECDSA_P384_PUBLIC signing_key;
	public SP_ECDSA_P384_PUBLIC secondary_key;
}

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1, Size = RKM_HDR_SIZE)]
struct key_manifest_hsp_rom_keys {
	public uint marker;
	public SP_ECDSA_P384_PUBLIC owner_key;
	public SP_ECDSA_P384_SIGNATURE signature;
	public key_manifest_hsp_rom_keys_header_signed header_signed;
	public key_manifest_hsp_rom_keys_data data;
}

// key_manifest_hsp_rom_dual_root.h

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
struct key_manifest_hsp_rom_dual_root_ownership_data {
	public SP_ECDSA_P384_PUBLIC new_authenticity_key;
	public SP_ECDSA_P384_PUBLIC new_authority_key;
}

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1, Size = OTM_HDR_SIZE_DUAL_ROOT)]
struct key_manifest_hsp_rom_dual_root_ownership {
	public uint marker;
	public SP_ECDSA_P384_PUBLIC authenticity_key;
	public SP_ECDSA_P384_PUBLIC authority_key;
	public SP_ECDSA_P384_SIGNATURE authenticity_signature;
	public SP_ECDSA_P384_SIGNATURE authority_signature;
	public key_manifest_hsp_rom_ownership_header_signed header_signed;
	public key_manifest_hsp_rom_dual_root_ownership_data data;
}

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
struct key_manifest_hsp_rom_dual_root_keys_data {
	public SP_ECDSA_P384_PUBLIC signing_key;
	public SP_ECDSA_P384_PUBLIC secondary_key;
}

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1, Size = RKM_HDR_SIZE_DUAL_ROOT)]
struct key_manifest_hsp_rom_dual_root_keys {
	public uint marker;
	public SP_ECDSA_P384_PUBLIC authenticity_key;
	public SP_ECDSA_P384_PUBLIC authority_key;
	public SP_ECDSA_P384_SIGNATURE authenticity_signature;
	public SP_ECDSA_P384_SIGNATURE authority_signature;
	public key_manifest_hsp_rom_keys_header_signed header_signed;
	public key_manifest_hsp_rom_dual_root_keys_data data;
}

// hsp_fw_1sp.h

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
struct hsp_fw_1sp_header_signed_header {
	public uint svn;
	[MarshalAs (UnmanagedType.ByValArray, SizeConst = 8)]
	public byte[] build_version;
	public uint load_addr;
	public uint length;
	public SP_MSG_384 digest;
}

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1, Size = SP1_HDR_SIZE)]
struct hsp_fw_1sp_header {
	public uint marker;
	public byte flags;
	[MarshalAs (UnmanagedType.ByValArray, SizeConst = 3)]
	private byte[] reserved;
	[MarshalAs (UnmanagedType.ByValArray, SizeConst = 16)]
	public byte[] iv;
	public SP_ECDSA_P384_SIGNATURE signature;
	public SP_ECDSA_P384_SIGNATURE secondary_signature;
	public hsp_fw_1sp_header_signed_header header_signed;
}

// key_manifest_hsp_firmware.h

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
struct key_manifest_hsp_firmware_manifest_signed {
	public ulong svn;
	public uint length;
	public SP_MSG_384 digest;
}

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
struct key_manifest_hsp_firmware_manifest_fw_key_slots {
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 10)]
	public DER_ECDSA_P384_PUBLIC[] slot;
};

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1, Size = FKM_HDR_SIZE)]
struct key_manifest_hsp_firmware_manifest {
	public uint marker;
	public DER_ECDSA_P384_SIGNATURE signature;
	public DER_ECDSA_P384_SIGNATURE secondary_sig;
	public key_manifest_hsp_firmware_manifest_signed header_signed;
	public key_manifest_hsp_firmware_manifest_fw_key_slots fw_key;
}

// Dual sign Firmware Key Manifest Layout
[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
struct key_manifest_hsp_firmware_manifest_header_prefix_dual_root {
	public byte     hdr_version;
	public byte		sig_block_type;
	public ushort 	hdr_size;
	public ushort	sig_block_size;
	public ushort	padding_length;
}

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
struct key_manifest_hsp_firmware_manifest_signed_dual_root {
	public uint marker;
	public key_manifest_hsp_firmware_manifest_header_prefix_dual_root header_prefix;
	public uint svn;
	public ulong build_version;
	public uint length;
	public SP_MSG_384 digest;
}

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
struct key_manifest_hsp_firmware_manifest_fw_key_slot_dual_root {
	public DER_ECDSA_P384_PUBLIC authenticity_key;
	public DER_ECDSA_P384_PUBLIC authority_key;
};

[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
struct key_manifest_hsp_firmware_manifest_fw_key_slots_dual_root {
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 10)]
	key_manifest_hsp_firmware_manifest_fw_key_slot_dual_root[] slot;
};


/* This layout is defined assuming padding length as zero.
   Reading of Manifest should be done after keeping Padding length */
[StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1, Size = FKM_HDR_SIZE_DUAL_ROOT)]
struct key_manifest_hsp_firmware_manifest_dual_root {
	public key_manifest_hsp_firmware_manifest_signed_dual_root header_signed;
	public DER_ECDSA_P384_SIGNATURE authenticity_signature;
	public DER_ECDSA_P384_SIGNATURE authority_signature;
	public key_manifest_hsp_firmware_manifest_fw_key_slots_dual_root fw_keys;
};

/* Input script arguments */

/**
 * A Windows path to a package image.
 */
static readonly string FILE_IMAGE = @"${FILE_IMAGE}";

/**
 * The target signing operation to execute.
 */
const long OPERATION = (${SIGNING_OPERATION:OPERATION_NONE}) & OPERATION_MASK;

/**
 * The key provider of the signing key.
 */
const KeyProvider SIGN_KEY_PROVIDER = KeyProvider.${SIGN_KEY_PROVIDER};

/**
 * The name of the signing key.
 */
static readonly string SIGN_KEY_NAME = @"${SIGN_KEY_NAME:}";

/**
 * The switch to use either HSM or test ECC key.
 */
const bool UseHsm = ${USE_HSM:true};

/**
 * The private ECC test key blob.
 */
public static byte[] SpSignEccPrivate = new byte []
{
	${ECC_PRIVATE_KEY:0}
};

/* Script utils */

// TODO: If HSM has a compatible framework version, use System.Formats.Asn1 to en/decode DER.

/**
 * Base DER object descriptor class.
 *
 * For the uses of this script, we are only expecting single byte length's in the tag.
 */
abstract class der_object {
	/**
	 * Instantiates a DER object descriptor.
	 *
	 * @param obj_type The object type identifier.
	 */
	protected der_object (byte obj_type)
	{
		object_type = obj_type;
	}

	/**
	 * The object type identifier.
	 */
	public byte object_type { get; private set; }

	/**
	 * The length of the data following the tag.
	 */
	public abstract int data_length { get; }

	/**
	 * The total length of the object.
	 */
	public int total_length
	{
		get {
			checked {
				return 2 + data_length;
			}
		}
	}

	/**
	 * Copies the object data to the DER byte buffer.
	 *
	 * @param buf The DER buffer.
	 * @param pos The current position in the DER buffer.
	 *
	 * @return The ending position of the DER byte stream.
	 */
	protected abstract int output_data (byte[] buf, int pos);

	/**
	 * Copies the tag and object data to the DER byte buffer.
	 *
	 * @param buf The DER buffer.
	 * @param pos The current position in the DER buffer.
	 *
	 * @return The ending position of the DER byte stream.
	 */
	public int output (byte[] buf, int pos)
	{
		buf[pos++] = object_type;


		byte len;
		checked {
			len = (byte) data_length;
		}
		buf[pos++] = len;

		return output_data (buf, pos);
	}
}

/**
 * A DER object used to hold multiple child DER objects.
 */
class der_container : der_object {
	/**
	 * Instantiates a DER container object.
	 *
	 * @param obj_type The object type identifier.
	 */
	public der_container (byte obj_type) : base (obj_type)
	{
	}

	/**
	 * A list of child DER objects.
	 */
	public readonly List<der_object> objects = new List<der_object> ();

	public override int data_length
	{
		get {
			int total = 0;
			foreach (var der_obj in objects) {
				var len = der_obj.total_length;
				checked {
					total += len;
				}
			}

			return total;
		}
	}

	protected override int output_data (byte[] buf, int pos)
	{
		foreach (var der_obj in objects) {
			pos = der_obj.output (buf, pos);
		}

		return pos;
	}
}

/**
 * A DER object representing a span of bytes.
 */
class der_byte_span : der_object {
	/**
	 * Instantiates a DER byte span object.
	 *
	 * @param obj_type The object type identifier.
	 * @param buf The data bytes.
	 * @param off The offset in buf where the object starts.
	 * @param len The length of the object in buf.
	 */
	public der_byte_span (byte obj_type, byte[] buf, int off, int len) : base (obj_type)
	{
		data = buf;
		offset = off;
		length = len;
	}

	public override int data_length => length;

	/**
	 * The source object data buffer.
	 */
	public byte[] data { get; private set; }

	/**
	 * The offset in data for the object.
	 */
	public int offset { get; protected set; }

	/**
	 * The length of the object in data.
	 */
	public int length { get; protected set; }

	protected override int output_data (byte[] buf, int pos)
	{
		Array.Copy (data, offset, buf, pos, length);
		return pos + length;
	}
}

/**
 * A DER object representing a big endian, unsigned big integer.
 */
class der_big_uint : der_byte_span {
	/**
	 * Instantiates a DER big uint object.
	 *
	 * @param buf The integer data bytes.
	 */
	public der_big_uint (byte[] buf) : base (0x02, buf, 0, buf.Length)
	{
		while ((length > 0) && (data[offset] == 0)) {
			--length;
			++offset;
		}
	}

	/**
	 * Flag that indicates if the MSB of the integer is set in data.
	 */
	public bool msb_set => (data[offset] & 0x80) != 0;

	public override int data_length
	{
		get {
			int len = length;
			if (msb_set) {
				checked {
					++len;
				}
			}

			return len;
		}
	}

	protected override int output_data (byte[] buf, int pos)
	{
		if (msb_set) {
			buf[pos++] = 0;
		}

		return base.output_data (buf, pos);
	}
}

/**
 * Decodes and extracts data from a DER byte stream.
 */
class der_decoder {
	/**
	 * Instantiates the DER decoder with a span of bytes.
	 *
	 * @param buf The DER byte stream buffer.
	 * @param pos The position in buf to start decoding.
	 * @param len The length of data to parse in buf.
	 */
	public der_decoder (byte[] buf, int pos, int len)
	{
		der = buf;
		offset = pos;
		length = len;
	}

	/**
	 * Instantiates the DER decoder with a byte array.
	 *
	 * @param buf The DER byte stream buffer.
	 */
	public der_decoder (byte[] buf) : this (buf, 0, buf.Length)
	{
	}

	/**
	 * The DER byte stream being parsed.
	 */
	public byte[] der { get; private set; }

	/**
	 * The offset in der being parsed.
	 */
	public int offset { get; private set; }

	/**
	 * The length of der being parsed.
	 */
	public int length { get; private set; }

	/**
	 * Validates the tag at the current position.
	 *
	 * @param len Returns the length of data for the current tag.
	 * See length note in der_object.
	 * @param type The expected tag type.
	 *
	 * @return true if the tag is valid, else false if there were any issues.
	 */
	public bool verify_tag (out byte len, byte type)
	{
		len = 0;

		if (length >= 0x80) {
			return false;
		}

		checked {
			length -= 2;
		}

		if (der[offset++] != type) {
			return false;
		}

		len = der[offset++];
		if (len > length) {
			return false;
		}

		return true;
	}

	/**
	 * Validates the tag at the current position.
	 *
	 * @param type The expected tag type.
	 *
	 * @return true if the tag is valid, else false if there were any issues.
	 */
	public bool verify_tag (byte type)
	{
		return verify_tag (out byte _, type);
	}

	/**
	 * Extracts the raw bytes of a big uint.
	 *
	 * @param int_bytes The buffer to copy the integer bytes to.
	 *
	 * @return true if the integer was parsed successfully, else false.
	 */
	public bool extract_big_uint (byte[] int_bytes)
	{
		if (!verify_tag (out byte len, 0x02)) {
			return false;
		}

		if (len > 0) {
			if ((der[offset] & 0x80) != 0) {
				return false;
			}

			checked {
				length -= len;
			}

			while ((len > 0) && (der[offset] == 0)) {
				--len;
				++offset;
			}
		}

		int pos = int_bytes.Length - len;
		for (int i = 0; i < pos; ++i) {
			int_bytes[i] = 0;
		}

		Array.Copy (der, offset, int_bytes, pos, len);
		offset += len;
		return true;
	}
}

/**
 * Ensures a marker matches the magic and throws an exception if it doesn't.
 *
 * @param marker The actual marker value.
 * @param magic The expected magic value.
 * @param error An error reason string to insert into the exception message.
 */
static void assert_marker (ulong marker, ulong magic, string error)
{
	if (marker != magic) {
		throw new Exception (string.Format ("{0}! expect=0x{1:X}, actual=0x{2:X}", error, magic,
			marker));
	}
}

/**
 * Ensures the signing operation is valid for the image type.
 *
 * @param marker The actual marker value.
 * @param magic The expected magic value.
 */
static void assert_operation (ulong marker, ulong magic)
{
	assert_marker (marker, magic, "Invalid signing operation for the image");
}

/**
 * Ensures the image is being parsed correctly by matching the header marker.
 *
 * @param marker The actual marker value.
 * @param magic The expected magic value.
 */
static void assert_header (ulong marker, ulong magic)
{
	assert_marker (marker, magic, "Unexpected marker");
}

/**
 * Gets the size of the managed struct type.
 *
 * @param T The struct type.
 * @return Size in bytes of the managed struct.
 */
static int struct_size<T> ()
{
	return Marshal.SizeOf (typeof (T));
}

/**
 * Ensures that the actual size of the struct is correct.
 *
 * @param T The struct type.
 */
static void assert_struct<T> (int size)
{
	int actual = struct_size<T> ();
	if (actual != size) {
		throw new Exception (string.Format (
			"Unexpected struct size! type={0}, expect={1}, actual={2}", typeof (T), size,
			actual));
	}
}

/**
 * Ensures the buffer is large enough to copy to/from a struct.
 *
 * @param T The struct type.
 * @param buffer The byte array.
 *
 * @return The size in bytes of the struct.
 */
static int assert_buffer<T> (byte[] buffer, bool exact_size) where T : struct
{
	int size = struct_size<T> ();

	bool valid;
	if (exact_size) {
		valid = (buffer.Length == size);
	}
	else {
		valid = (buffer.Length >= size);
	}
	if (!valid) {
		throw new Exception (string.Format (
			"Unexpected buffer size for the struct! exact_required={0}, buf={1}, struct={2}",
			exact_size, buffer.Length, size));
	}

	return size;
}

/**
 * Allocates a marshaled structure.
 *
 * @param T The struct type.
 * @param obj The struct object to return.
 */
static void alloc_struct<T> (out T obj)
{
	IntPtr ptr = Marshal.AllocHGlobal (struct_size<T> ());
	try {
		obj = (T) Marshal.PtrToStructure (ptr, typeof (T));
	}
	finally {
		Marshal.FreeHGlobal (ptr);
	}
}

/**
 * Copies the bytes at the beginning of a byte array to a structure.
 *
 * @param T The struct type.
 * @param obj The struct object to copy to.
 * @param data The byte array.
 */
static void bytes_to_struct<T> (out T obj, byte[] data, bool exact_size) where T : struct
{
	assert_buffer<T> (data, exact_size);
	GCHandle hData = GCHandle.Alloc (data, GCHandleType.Pinned);
	try {
		obj = (T) Marshal.PtrToStructure (hData.AddrOfPinnedObject (), typeof (T));
	}
	finally {
		hData.Free ();
	}
}

/**
 * Copies the contents of a structure to the beginning of a byte array.
 *
 * @param T The struct type.
 * @param data The byte array.
 * @param obj The struct object to copy from.
 *
 * @return The size in bytes of the struct.
 */
static int struct_to_bytes<T> (byte[] buf, ref T obj, bool exact_size) where T : struct
{
	int size = assert_buffer<T> (buf, exact_size);
	GCHandle hBuf = GCHandle.Alloc (buf, GCHandleType.Pinned);
	try {
		Marshal.StructureToPtr (obj, hBuf.AddrOfPinnedObject (), false);
	}
	finally {
		hBuf.Free ();
	}
	return size;
}

/**
 * Creates a byte array with the contents of a struct.
 *
 * @param T The struct type.
 * @param obj The struct object to copy from.
 *
 * @return The allocated byte array.
 */
static byte[] struct_to_bytes<T> (ref T obj) where T : struct
{
	byte[] buf = new byte[struct_size<T> ()];
	struct_to_bytes (buf, ref obj, true);
	return buf;
}

/**
 * Reads a struct from a BinaryReader at a specified stream position.
 *
 * @param T The struct type.
 * @param obj The struct instance to read to.
 * @param reader The BinaryReader instance.
 * @param stream_pos The position in the stream to seek to before reading.
 * @param buf The intermediate I/O buffer.
 *
 * @return The intermediate I/O buffer.
 */
static byte[] read_struct<T> (out T obj, BinaryReader reader, long stream_pos, byte[] buf)
	where T : struct
{
	int size = struct_size<T> ();
	if (buf == null) {
		buf = new byte[size];
	}

	reader.BaseStream.Position = stream_pos;
	reader.Read (buf, 0, size);
	bytes_to_struct (out obj, buf, false);

	return buf;
}

/**
 * Reads a Firmware Key Manifest struct from a BinaryReader.
 * @param reader The BinaryReader instance.
 * @param buf The intermediate I/O buffer.
 *
 * @return The Firmware Key Manifest read from buffer.
 */
static key_manifest_hsp_firmware_manifest_dual_root read_struct_fkm (BinaryReader reader, byte[] buf)
{
	var fkm_manifest = new key_manifest_hsp_firmware_manifest_dual_root();

	/* Read the common prefix header & FW Key Manifest Header  */
	read_struct (out fkm_manifest.header_signed, reader,0, buf);

	/* Read header size, signature block size & Padding length */
	ushort hdr_size = fkm_manifest.header_signed.header_prefix.hdr_size;
	ushort sig_block_size = fkm_manifest.header_signed.header_prefix.sig_block_size;
	ushort padding_length = fkm_manifest.header_signed.header_prefix.padding_length;

	/* Read Authenticity Signature */
	read_struct (out fkm_manifest.authenticity_signature, reader, hdr_size, buf);

	/* Read Authority Signature */
	read_struct (out fkm_manifest.authority_signature, reader, (hdr_size + (sig_block_size/2)), buf);

	/* Read FW Keys */
	read_struct (out fkm_manifest.fw_keys, reader, (hdr_size + sig_block_size + padding_length), buf);

	return fkm_manifest;
}

/**
 * Writes a struct to a BinaryWriter at a specified stream position.
 *
 * @param T The struct type.
 * @param writer The BinaryWriter instance.
 * @param stream_pos The position in the stream to seek to before reading.
 * @param obj The struct instance to write to the stream.
 * @param buf The intermediate I/O buffer.
 *
 * @return The number of bytes written to the stream.
 */
static int write_struct<T> (BinaryWriter writer, long stream_pos, ref T obj, byte[] buf)
	where T : struct
{
	writer.BaseStream.Position = stream_pos;
	int size = struct_to_bytes (buf, ref obj, false);
	writer.Write (buf, 0, size);
	return size;
}

/**
 * Writes a struct to a BinaryWriter at a specified stream position.
 *
 * @param T The struct type.
 * @param writer The BinaryWriter instance.
 * @param stream_pos The position in the stream to seek to before reading.
 * @param obj The struct instance to write to the stream.
 * @param buf The intermediate I/O buffer.
 *
 * @return The number of bytes written to the stream.
 */
static void write_struct_fkm (BinaryWriter writer, key_manifest_hsp_firmware_manifest_dual_root fkm_manifest, byte[] buf)
{
	/* Read header size, signature block size & Padding length */
	ushort hdr_size = fkm_manifest.header_signed.header_prefix.hdr_size;
	ushort sig_block_size = fkm_manifest.header_signed.header_prefix.sig_block_size;

	/* Write Authenticity Signature */
	write_struct (writer, hdr_size, ref fkm_manifest.authenticity_signature, buf);

	/* Write Authority Signature */
	write_struct (writer, (hdr_size + (sig_block_size/2)), ref fkm_manifest.authority_signature, buf);
}

/**
 * Computes a hash of a struct and returns the byte array containing the digest.
 *
 * @param T The input data struct type.
 * @param obj The struct instance to compute the hash over.
 *
 * @return The digest byte array.
 */
static byte[] hash_struct<T> (ref T obj) where T : struct
{
	byte[] data = struct_to_bytes (ref obj);
	byte[] hash = SpUtil.ComputeHash (HashAlgo.Sha384, data);
	return hash;
}

/**
 * Computes a hash of a struct and copies the digest to a result struct.
 *
 * @param T The input data struct type.
 * @param digest The digest buffer to save the computed hash to.
 * @param obj The struct instance to compute the hash over.
 */
static void hash_struct<T> (out SP_MSG_384 digest, ref T obj) where T : struct
{
	Console.WriteLine ("Computing hash...");
	byte[] hash = hash_struct (ref obj);
	bytes_to_struct (out digest, hash, true);
}

/**
 * Evaluates the assigned script operations against an input operation mask.
 *
 * @param op_mask The input operation mask to test.
 *
 * @return true if any of the op_mask bits are set in OPERATION, else false.
 */
static bool test_op (long op_mask)
{
	return (OPERATION & op_mask) != 0;
}

/**
 * Signs a struct with the script signing key.
 *
 * @param T The input data struct type.
 * @param signature The signature buffer to save the signature to.
 * @param obj THe struct object to compute a signature over.
 */
static void sign_struct<T> (out SP_ECDSA_P384_SIGNATURE signature, ref T obj)
	where T : struct
{
	byte[] hash = hash_struct (ref obj);
	byte[] sig = null;

	Console.WriteLine ("Signing hash...");
	if (UseHsm) {
		sig = SpUtil.SignHash (SIGN_KEY_PROVIDER, SIGN_KEY_NAME, HashAlgo.Sha384, hash, false);
	}
	else {
		sig = SpUtil.SignHash (SIGN_KEY_PROVIDER, SpSignEccPrivate, HashAlgo.Sha384, hash, false);
	}
	bytes_to_struct (out signature, sig, true);
}

/**
 * Obtains the raw public key blob of the signing key.
 *
 * @return A byte array that contains the public key in the form of SP_ECDSA_P384_PUBLIC_SIZE.
 */
static byte[] get_pub_key ()
{
	byte[] key_blob = null;

	if (UseHsm) {
		key_blob = SpUtil.ExportPublicKeyData (SIGN_KEY_PROVIDER, SIGN_KEY_NAME);
		key_blob = SpUtil.RemoveKeyDataMagic (key_blob);
	}
	else {
		key_blob = new byte[SP_ECDSA_P384_PUBLIC_SIZE];
		Array.Copy(SpSignEccPrivate, 8, key_blob, 0, SP_ECDSA_P384_PUBLIC_SIZE);
	}
	return key_blob;
}

/**
 * Exports the public key of the signing key.
 *
 * @param signer The key buffer to store the key to.
 */
static void save_pub_key (out SP_ECDSA_P384_PUBLIC signer)
{
	Console.WriteLine ("Saving public key...");
	bytes_to_struct (out signer, get_pub_key (), true);
}

/**
 * Signs a struct with the script signing key and saves the public key.
 *
 * @param T The input data struct type.
 * @param signer The public key buffer to save the signing key to.
 * @param signature The signature buffer to save the signature to.
 * @param obj THe struct object to compute a signature over.
 */
static void sign_struct<T> (out SP_ECDSA_P384_PUBLIC signer, out SP_ECDSA_P384_SIGNATURE signature,
	ref T obj) where T : struct
{
	save_pub_key (out signer);
	sign_struct (out signature, ref obj);
}

/**
 * Signs a struct with the script signing key and encodes the signature using ASN.1/DER.
 *
 * @param T The input data struct type.
 * @param signature The signature buffer to save the signature to.
 * @param obj THe struct object to compute a signature over.
 */
static void sign_struct<T> (ref DER_ECDSA_P384_SIGNATURE signature, ref T obj) where T : struct
{
	sign_struct (out SP_ECDSA_P384_SIGNATURE sig, ref obj);
	var der_sig = new der_container (0x30);
	der_sig.objects.Add (new der_big_uint (sig.R.data));
	der_sig.objects.Add (new der_big_uint (sig.S.data));
	der_sig.output (signature.data, 0);
}

/**
 * Prints information to the console if verification failed.
 *
 * @param type The type of verification being performed.
 * @param name The name of the object being verified.
 * @param valid The result of the verification.
 */
static void verify_inform (string type, string name, bool valid)
{
	if (!valid) {
		Console.WriteLine ("{0} verification failed! ({1})", type, name, valid);
	}
}

/**
 * Compares 2 byte arrays and prints information if it doesn't match.
 *
 * @param type The type of verification being performed.
 * @param name The name of the object being verified.
 * @param recorded The actual bytes written to the image.
 * @param calculated The actual calculated value expected to match the recorded value.
 *
 * @return true if the bytes match, else false.
 */
static bool verify_bytes (string type, string name, byte[] recorded, byte[] calculated)
{
	var valid = SpUtil.BytesEqual (recorded, calculated);
	verify_inform (type, name, valid);
	if (!valid) {
		Console.WriteLine ("RECORDED:");
		SpUtil.PrintBytes (recorded);
		Console.WriteLine ("CALCULATED:");
		SpUtil.PrintBytes (calculated);
	}

	return valid;
}

/**
 * Verifies a recorded hash matches the digest of an input data struct.
 *
 * @param T The input data struct type.
 * @param hash_name The name of the object being verified
 * @param digest The recorded hash digest.
 * @param data The input data being hashed.
 *
 * @return True if the computed hash matches the recorded digest, else false.
 */
static bool verify_hash<T> (string hash_name, ref SP_MSG_384 digest, ref T data) where T : struct
{
	var recorded = digest.data;
	var calculated = hash_struct (ref data);
	return verify_bytes ("Hash", hash_name, recorded, calculated);
}

/**
 * Verifies a recorded signature is valid for an input data struct.
 *
 * @param T The input data struct type.
 * @param sig_name The name of the object being verified.
 * @param key The public key that signed the object.
 * @param signature The recorded signature.
 * @param data The input data being signed.
 *
 * @return True if the recorded signature is valid, else false.
 */
static bool verify_sig<T> (string sig_name, ref SP_ECDSA_P384_PUBLIC key,
	ref SP_ECDSA_P384_SIGNATURE signature, ref T data) where T : struct
{
	var valid = false;
	var sig_bytes = struct_to_bytes (ref signature);

	var digest = hash_struct (ref data);
	var key_blob = struct_to_bytes (ref key);
	key_blob = SpUtil.AddKeyDataMagic (KeyProvider.EccPublicBlob, key_blob);

	valid = SpUtil.VerifyHash (KeyProvider.EccPublicBlob, key_blob, HashAlgo.Sha384, digest,
			sig_bytes, false);

	verify_inform ("Signature", sig_name, valid);
	return valid;
}

/**
 * Verifies a recorded signature is valid for an input data struct using the script signing key.
 *
 * @param T The input data struct type.
 * @param sig_name The name of the object being verified.
 * @param signature The recorded signature.
 * @param data The input data being signed.
 *
 * @return True if the recorded signature is valid, else false.
 */
static bool verify_sig<T> (string sig_name, ref DER_ECDSA_P384_SIGNATURE signature, ref T data)
	where T : struct
{
	byte[] sig_bytes = null;
	{
		alloc_struct (out SP_ECDSA_P384_SIGNATURE sig);

		Console.WriteLine ("{0}: Parsing DER signature...", sig_name);

		var decoder = new der_decoder (signature.data);

		if (!decoder.verify_tag (0x30)) {
			return false;
		}

		if (!decoder.extract_big_uint (sig.R.data)) {
			return false;
		}

		if (!decoder.extract_big_uint (sig.S.data)) {
			return false;
		}

		sig_bytes = struct_to_bytes (ref sig);
	}

	var digest = hash_struct (ref data);
	var valid = false;

	if (UseHsm) {
		valid = SpUtil.VerifyHash (SIGN_KEY_PROVIDER, SIGN_KEY_NAME, HashAlgo.Sha384, digest,
					sig_bytes, false);
	}
	else {
		var key_blob = SpUtil.AddKeyDataMagic (KeyProvider.EccPublicBlob, SpSignEccPrivate);
		valid = SpUtil.VerifyHash (KeyProvider.EccPublicBlob, key_blob, HashAlgo.Sha384, digest,
				sig_bytes, false);
	}
	verify_inform ("Signature", sig_name, valid);
	return valid;
}

/**
 * Verifies a header consisting of a hashed data structure and a signed data structure.
 *
 * @param T1 The input signature data struct type.
 * @param T2 The input hash data struct type.
 * @param name The name of the object being verified.
 * @param key The public key that signed the object.
 * @param signature the recorded signature.
 * @param sig_data The input data being signed.
 * @param digest The recorded hash digest.
 * @param hash_data The input data being hashed.
 *
 * @return true if both the hash and signature are valid, else false.
 */
static bool verify_header<T1, T2> (string name, ref SP_ECDSA_P384_PUBLIC key,
	ref SP_ECDSA_P384_SIGNATURE signature, ref T1 sig_data, ref SP_MSG_384 digest,
	ref T2 hash_data) where T1 : struct where T2 : struct
{
	if (!verify_hash (name, ref digest, ref hash_data)) {
		return false;
	}

	return verify_sig (name, ref key, ref signature, ref sig_data);
}

/**
 * Verifies that the recorded key matches the signing key.
 *
 * @param key_name The name of the key being compared.
 * @param key The structure containing the key contents.
 *
 * @return true if the recorded key matches the signing key, else false.
 */
static bool verify_key (string key_name, ref SP_ECDSA_P384_PUBLIC key)
{
	var recorded = struct_to_bytes (ref key);
	var calculated = get_pub_key ();
	return verify_bytes ("Public key", key_name, recorded, calculated);
}


/* Struct conversion correctness */

assert_struct<SP_MSG_384> (SP_MSG_384_SIZE);
assert_struct<SP_ECDSA_P384_PUBLIC> (SP_ECDSA_P384_PUBLIC_SIZE);
assert_struct<SP_ECDSA_P384_SIGNATURE> (SP_ECDSA_P384_SIGNATURE_SIZE);
assert_struct<DER_ECDSA_P384_PUBLIC> (DER_ECDSA_P384_PUBLIC_MAX_SIZE);
assert_struct<key_manifest_hsp_rom_grant> (TGM_HDR_SIZE);
assert_struct<key_manifest_hsp_rom_ownership> (OTM_HDR_SIZE);
assert_struct<key_manifest_hsp_rom_dual_root_ownership> (OTM_HDR_SIZE_DUAL_ROOT);
assert_struct<key_manifest_hsp_rom_keys> (RKM_HDR_SIZE);
assert_struct<key_manifest_hsp_rom_dual_root_keys> (RKM_HDR_SIZE_DUAL_ROOT);
assert_struct<hsp_fw_1sp_header> (SP1_HDR_SIZE);
assert_struct<key_manifest_hsp_firmware_manifest> (FKM_HDR_SIZE);
assert_struct<key_manifest_hsp_firmware_manifest_dual_root> (FKM_HDR_SIZE_DUAL_ROOT);


/* Script execution */

/* Usage checks. */
if (test_op (SP1_OPERATION_MASK)) {
	// FW key manifest is not a 1SP image component
	if (test_op (FW_KEYS_OPERATION_MASK)) {
		throw new Exception ("Invalid signing operation!");
	}

	// Tenancy and ownership transfer are separate types of images
	if (test_op (TRANSFER_OPERATION_MASK) && test_op (TENANT_OPERATION_MASK)) {
		throw new Exception ("Invalid signing operation!");
	}
}

Console.WriteLine ("Package Image:");
Console.WriteLine (FILE_IMAGE);

if (!test_op (OPERATION_MASK)) {
	return 0;
}

bool update = test_op (OPERATION_UPDATE_MASK);
using (var stream = File.Open (FILE_IMAGE, FileMode.Open,
	update ? FileAccess.ReadWrite : FileAccess.Read)) {
	using (var reader = new BinaryReader (stream)) {

		BinaryWriter writer = null;
		if (update) {
			writer = new BinaryWriter (stream);
		}

		var buffer = new byte[IO_BUF_SIZE];

		stream.Position = 0;
		ulong pkg_magic = reader.ReadUInt32 ();

		if (pkg_magic == FKM_MAGIC) {
			if (test_op (FW_KEYS_OPERATION_MASK)) {
				var manifest = new key_manifest_hsp_firmware_manifest ();

				Console.WriteLine ("Reading firmware key manifest...");
				read_struct (out manifest, reader, 0, buffer);

				if (test_op (OPERATION_SIGN_FW_KEYS_PRIMARY | OPERATION_SIGN_FW_KEYS_SECONDARY)) {
					Console.WriteLine ("Hashing the FW keys...");
					hash_struct (out manifest.header_signed.digest, ref manifest.fw_key);

					if (test_op (OPERATION_SIGN_FW_KEYS_SECONDARY)) {
						Console.WriteLine ("Signing FW key manifest (secondary)...");
						sign_struct (ref manifest.secondary_sig, ref manifest.header_signed);
					}

					if (test_op (OPERATION_SIGN_FW_KEYS_PRIMARY)) {
						Console.WriteLine ("Signing FW key manifest (primary)...");
						sign_struct (ref manifest.signature, ref manifest.header_signed);
					}
				}

				if (test_op (OPERATION_VERIFY_FW_KEYS_PRIMARY |
					OPERATION_VERIFY_FW_KEYS_SECONDARY)) {
					if (!verify_hash ("FW keys digest", ref manifest.header_signed.digest,
						ref manifest.fw_key)) {
						return 1;
					}

					if (test_op (OPERATION_VERIFY_FW_KEYS_SECONDARY) && !verify_sig (
						"FW keys secondary signature", ref manifest.secondary_sig,
						ref manifest.header_signed)) {
						return 1;
					}

					if (test_op (OPERATION_VERIFY_FW_KEYS_PRIMARY) && !verify_sig (
						"FW keys primary signature", ref manifest.signature,
						ref manifest.header_signed)) {
						return 1;
					}
				}

				if (test_op (FW_KEYS_OPERATION_UPDATE)) {
					Console.WriteLine ("Writing FW key manifest...");
					write_struct (writer, 0, ref manifest, buffer);
				}
			}
		}
		else if (pkg_magic == FKM_MAGIC_V2) {
			if (test_op (FW_KEYS_OPERATION_MASK)) {
				ushort padding_length = 0;
				var manifest_dual_root = new key_manifest_hsp_firmware_manifest_dual_root ();

				Console.WriteLine ("Reading firmware key manifest for dual signed image...");
				manifest_dual_root = read_struct_fkm (reader, buffer);

				if (test_op (OPERATION_SIGN_FW_KEYS_PRIMARY |
				    OPERATION_SIGN_FW_KEYS_SECONDARY)) {
					Console.WriteLine ("Hashing the FW keys...");
					hash_struct (out manifest_dual_root.header_signed.digest, ref manifest_dual_root.fw_keys);

					if (test_op (OPERATION_SIGN_FW_KEYS_SECONDARY)) {
						Console.WriteLine ("Signing FW key manifest (secondary)...");
						sign_struct (ref manifest_dual_root.authority_signature, ref manifest_dual_root.header_signed);
					}

					if (test_op (OPERATION_SIGN_FW_KEYS_PRIMARY)) {
						Console.WriteLine ("Signing FW key manifest (primary)...");
						sign_struct (ref manifest_dual_root.authenticity_signature, ref manifest_dual_root.header_signed);
					}
				}

				if (test_op (OPERATION_VERIFY_FW_KEYS_PRIMARY |
					OPERATION_VERIFY_FW_KEYS_SECONDARY)) {
						if (!verify_hash ("FW keys digest", ref manifest_dual_root.header_signed.digest,
							ref manifest_dual_root.fw_keys)) {
							return 1;
						}

					if (test_op (OPERATION_VERIFY_FW_KEYS_SECONDARY)) {
						if (!verify_sig ("FW keys secondary signature",
							ref manifest_dual_root.authority_signature,
							ref manifest_dual_root.header_signed)) {
							return 1;
						}
					}

					if (test_op (OPERATION_VERIFY_FW_KEYS_PRIMARY)) {
						if (!verify_sig ("FW keys primary signature",
							ref manifest_dual_root.authenticity_signature,
							ref manifest_dual_root.header_signed)) {
							return 1;
						}
					}
				}
				if (test_op (FW_KEYS_OPERATION_UPDATE)) {
					Console.WriteLine ("Writing FW key manifest for dual signed image...");
					write_struct_fkm (writer, manifest_dual_root, buffer);
				}
			}
		}
		else {
			long rkm_base = 0;
			bool single_root_manifest;
			ulong rkm_populated_magic = 0;
			switch ((uint) pkg_magic) {
				case NUL_MAGIC:
					rkm_base = NUL_HDR_SIZE;
					break;

				case OTM_MAGIC:
					rkm_base = struct_size<key_manifest_hsp_rom_ownership> ();
					break;

				case OTM_MAGIC_DUAL_ROOT:
					rkm_base = struct_size<key_manifest_hsp_rom_dual_root_ownership> ();
					break;

				case TGM_MAGIC:
					rkm_base = struct_size<key_manifest_hsp_rom_grant> ();
					break;

				default:
					throw new Exception ("Invalid package image!");
			}

			// Determine if this is a single root or dual root manifest.
			reader.BaseStream.Position = rkm_base;
			rkm_populated_magic = reader.ReadUInt32 ();

			switch (rkm_populated_magic) {
				case RKM_MAGIC:
					single_root_manifest = true;
					break;

				case RKM_MAGIC_DUAL_ROOT:
					single_root_manifest = false;
					break;

				default:
					throw new Exception ("Invalid Root Manifest Magic Number!");
			}

			long fw_base = 0;

			if (single_root_manifest) {
				fw_base = rkm_base + struct_size<key_manifest_hsp_rom_keys> ();
			}
			else {
				fw_base = rkm_base + struct_size<key_manifest_hsp_rom_dual_root_keys> ();
			}

			var manifest = new key_manifest_hsp_rom_keys ();
			var manifest_dual_root = new key_manifest_hsp_rom_dual_root_keys ();
			var grant = new key_manifest_hsp_rom_grant ();
			var ownership = new key_manifest_hsp_rom_ownership ();
			var ownership_dual_root = new key_manifest_hsp_rom_dual_root_ownership ();
			var firmware = new hsp_fw_1sp_header ();

			// Read all the necessary headers

			if (test_op (TENANT_OPERATION_MASK) || (test_op (OPERATION_VERIFY_FIRMWARE) &&
				(pkg_magic == TGM_MAGIC))) {
				assert_operation (pkg_magic, TGM_MAGIC);

				Console.WriteLine ("Reading tenancy grant...");
				read_struct (out grant, reader, 0, buffer);
			}
			else if (test_op (TRANSFER_OPERATION_MASK)) {
				uint owner_magic = (single_root_manifest) ? OTM_MAGIC : OTM_MAGIC_DUAL_ROOT;
				assert_operation (pkg_magic, owner_magic);

				Console.WriteLine ("Reading ownership transfer...");

				if (single_root_manifest) {
					read_struct (out ownership, reader, 0, buffer);
				}
				else {
					read_struct (out ownership_dual_root, reader, 0, buffer);
				}
			}

			if (test_op (MANIFEST_OPERATION_MASK)) {
				Console.WriteLine ("Reading key manifest...");
				if (single_root_manifest) {
					read_struct (out manifest, reader, rkm_base, buffer);

					assert_header (manifest.marker, RKM_MAGIC);
				}
				else {
					read_struct (out manifest_dual_root, reader, rkm_base, buffer);

					assert_header (manifest_dual_root.marker, RKM_MAGIC_DUAL_ROOT);
				}
			}

			if (test_op (FIRMWARE_OPERATION_MASK)) {
				Console.WriteLine ("Reading firmware header...");
				read_struct (out firmware, reader, fw_base, buffer);

				assert_header (firmware.marker, SP1_MAGIC);
			}

			// Do operations from bottom up for higher ops that are dependent on lower ops

			if (test_op (FIRMWARE_OPERATION_MASK)) {
				if (test_op (OPERATION_SIGN_FIRMWARE_SECONDARY)) {
					Console.WriteLine ("Signing firmware (secondary)...");
					sign_struct (out firmware.secondary_signature, ref firmware.header_signed);
				}

				if (test_op (OPERATION_SIGN_FIRMWARE_PRIMARY)) {
					Console.WriteLine ("Signing firmware (primary)...");
					sign_struct (out firmware.signature, ref firmware.header_signed);
				}
			}

			if (test_op (TENANT_OPERATION_MASK)) {
				if (test_op (OPERATION_TENANT_SET_KEY)) {
					Console.WriteLine ("Saving tenant key...");
					save_pub_key (out grant.data.tenant_key);
				}

				if (test_op (OPERATION_SIGN_TENANCY)) {
					Console.WriteLine ("Hashing the tenancy grant...");
					hash_struct (out grant.header_signed.digest, ref grant.data);

					Console.WriteLine ("Signing the tenancy grant...");
					sign_struct (out grant.signature, ref grant.header_signed);
				}

				if (test_op (TENANT_OPERATION_VERIFY)) {
					Console.WriteLine ("Verifying tenancy grant...");
					if (test_op (OPERATION_VERIFY_TENANT_KEY)) {
						if (!verify_key ("Tenant key", ref grant.data.tenant_key)) {
							return 1;
						}
					}

					if (test_op (OPERATION_VERIFY_TENANCY)) {
						if (single_root_manifest) {
							if (!verify_header ("Tenancy grant", ref manifest.data.signing_key,
								ref grant.signature, ref grant.header_signed,
								ref grant.header_signed.digest, ref grant.data)) {
								return 1;
							}
						}
						else {
							if (!verify_header ("Tenancy grant", ref manifest_dual_root.data.signing_key,
								ref grant.signature, ref grant.header_signed,
								ref grant.header_signed.digest, ref grant.data)) {
								return 1;
							}
						}
					}
				}
			}
			else if (test_op (TRANSFER_OPERATION_MASK)) {
				if (test_op (OPERATION_TRANSFER_SET_NEW_OWNER)) {
					Console.WriteLine ("Saving new authenticity owner key...");
					if (single_root_manifest) {
						save_pub_key (out ownership.data.new_owner_key);
					}
					else {
						save_pub_key (out ownership_dual_root.data.new_authenticity_key);
					}
				}

				if (test_op (OPERATION_TRANSFER_SET_NEW_OWNER_SECONDARY)) {
					Console.WriteLine ("Saving new authority owner key...");
					if (single_root_manifest) {
						throw new Exception ("Single root manifest does not include authority key!");
					}
					else {
						save_pub_key (out ownership_dual_root.data.new_authority_key);
					}
				}

				if (test_op (OPERATION_SIGN_TRANSFER_MANIFEST)) {
					if (single_root_manifest) {
						Console.WriteLine ("Saving old authenticity owner key...");
						save_pub_key (out ownership.owner_key);

						Console.WriteLine ("Hashing the ownership transfer...");
						hash_struct (out ownership.header_signed.digest, ref ownership.data);

						Console.WriteLine ("Signing the ownership transfer with authenticity key...");
						sign_struct (out ownership.signature, ref ownership.header_signed);
					}
					else {
						Console.WriteLine ("Saving old authenticity owner key...");
						save_pub_key (out ownership_dual_root.authenticity_key);

						Console.WriteLine ("Hashing the ownership transfer...");
						hash_struct (out ownership_dual_root.header_signed.digest, ref ownership_dual_root.data);

						Console.WriteLine ("Signing the ownership transfer with authenticity key...");
						sign_struct (out ownership_dual_root.authenticity_signature, ref ownership_dual_root.header_signed);
					}
				}

				if (test_op (OPERATION_SIGN_TRANSFER_MANIFEST_SECONDARY)) {
					if (single_root_manifest) {
						throw new Exception ("Single root manifest does not include authority key!");
					}
					else {
						Console.WriteLine ("Saving old authority owner key...");
						save_pub_key (out ownership_dual_root.authority_key);

						Console.WriteLine ("Hashing the ownership transfer...");
						hash_struct (out ownership_dual_root.header_signed.digest, ref ownership_dual_root.data);

						Console.WriteLine ("Signing the ownership transfer with authority key...");
						sign_struct (out ownership_dual_root.authority_signature, ref ownership_dual_root.header_signed);
					}
				}

				if (test_op (TRANSFER_OPERATION_VERIFY)) {
					Console.WriteLine ("Verifying ownership transfer...");
					if (test_op (OPERATION_VERIFY_OLD_OWNER_KEY)) {
						if (single_root_manifest) {
							if (!verify_key ("Old Authenticity Key", ref ownership.owner_key)) {
								return 1;
							}
						}
						else {
							if (!verify_key ("Old Authenticity Key", ref ownership_dual_root.authenticity_key)) {
								return 1;
							}
						}
					}

					if (test_op (OPERATION_VERIFY_OLD_OWNER_KEY_SECONDARY)) {
						if (single_root_manifest) {
							throw new Exception ("Single root manifest does not include authority key!");
						}
						else {
							if (!verify_key ("Old Authority Key", ref ownership_dual_root.authority_key)) {
								return 1;
							}
						}
					}

					if (test_op (OPERATION_VERIFY_NEW_OWNER_KEY)) {
						if (single_root_manifest) {
							if (!verify_key ("New Authenticity Key", ref ownership.data.new_owner_key)) {
								return 1;
							}
						}
						else {
							if (!verify_key ("New Authenticity Key", ref ownership_dual_root.data.new_authenticity_key)) {
								return 1;
							}
						}
					}

					if (test_op (OPERATION_VERIFY_NEW_OWNER_KEY_SECONDARY)) {
						if (single_root_manifest) {
							throw new Exception ("Single root manifest does not include authority key!");
						}
						else {
							if (!verify_key ("New Authority Key", ref ownership_dual_root.data.new_authority_key)) {
								return 1;
							}
						}
					}

					if (test_op (OPERATION_VERIFY_TRANSFER)) {
						if (single_root_manifest) {
							if (!verify_header ("Owner transfer Authenticity Key", ref ownership.owner_key,
								ref ownership.signature, ref ownership.header_signed,
								ref ownership.header_signed.digest, ref ownership.data)) {
								return 1;
							}
						}
						else {
							if (!verify_header ("Owner transfer Authenticity Key", ref ownership_dual_root.authenticity_key,
								ref ownership_dual_root.authenticity_signature, ref ownership_dual_root.header_signed,
								ref ownership_dual_root.header_signed.digest, ref ownership_dual_root.data)) {
								return 1;
							}
						}
					}

					if (test_op (OPERATION_VERIFY_TRANSFER_SECONDARY)) {
						if (single_root_manifest) {
							throw new Exception ("Single root manifest does not include authority key!");
						}
						else {
							if (!verify_header ("Owner transfer Authority Key", ref ownership_dual_root.authority_key,
								ref ownership_dual_root.authority_signature, ref ownership_dual_root.header_signed,
								ref ownership_dual_root.header_signed.digest, ref ownership_dual_root.data)) {
								return 1;
							}
						}
					}
				}
			}

			if (test_op (MANIFEST_OPERATION_MASK)) {
				if (test_op (OPERATION_MANIFEST_SET_SECONDARY_KEY)) {
					Console.WriteLine ("Saving the manifest secondary key...");
					if (single_root_manifest) {
						save_pub_key (out manifest.data.secondary_key);

						manifest.header_signed.valid_keys = 2;
					}
					else {
						save_pub_key (out manifest_dual_root.data.secondary_key);

						manifest_dual_root.header_signed.valid_keys = 2;
					}
				}

				if (test_op (OPERATION_MANIFEST_SET_SIGNING_KEY)) {
					Console.WriteLine ("Saving the manifest signing key...");
					if (single_root_manifest) {
						save_pub_key (out manifest.data.signing_key);
					}
					else {
						save_pub_key (out manifest_dual_root.data.signing_key);
					}
				}

				if (test_op (OPERATION_MANIFEST_SET_ROOT_KEY)) {
					Console.WriteLine ("Saving the manifest authenticity owner key...");
					if (single_root_manifest) {
						save_pub_key (out manifest.owner_key);
					}
					else {
						save_pub_key (out manifest_dual_root.authenticity_key);
					}
				}

				if (test_op (OPERATION_MANIFEST_SET_ROOT_KEY_SECONDARY)) {
					Console.WriteLine ("Saving the manifest authority owner key...");
					if (single_root_manifest) {
						throw new Exception ("Single root manifest does not include authority key!");
					}
					else {
						save_pub_key (out manifest_dual_root.authority_key);
					}
				}

				if (test_op (OPERATION_SIGN_ROOT_KEY_MANIFEST)) {
					Console.WriteLine ("Hashing the key manifest...");
					if (single_root_manifest) {
						hash_struct (out manifest.header_signed.digest, ref manifest.data);

						Console.WriteLine ("Signing the key manifest with Authenticity Key...");
						sign_struct (out manifest.signature, ref manifest.header_signed);
					}
					else {
						hash_struct (out manifest_dual_root.header_signed.digest, ref manifest_dual_root.data);

						Console.WriteLine ("Signing the key manifest with Authenticity Key...");
						sign_struct (out manifest_dual_root.authenticity_signature, ref manifest_dual_root.header_signed);
					}
				}

				if (test_op (OPERATION_SIGN_ROOT_KEY_MANIFEST_SECONDARY)) {
					Console.WriteLine ("Hashing the key manifest...");
					if (single_root_manifest) {
						throw new Exception ("Single root manifest does not include authority key!");
					}
					else {
						hash_struct (out manifest_dual_root.header_signed.digest, ref manifest_dual_root.data);

						Console.WriteLine ("Signing the key manifest with Authority Key...");
						sign_struct (out manifest_dual_root.authority_signature, ref manifest_dual_root.header_signed);
					}
				}

				if (test_op (MANIFEST_OPERATION_VERIFY)) {
					Console.WriteLine ("Verifying key manifest...");
					if (test_op (OPERATION_VERIFY_ROOT_KEY)) {
						if (single_root_manifest) {
							if (!verify_key ("Authenticity Root", ref manifest.owner_key)) {
								return 1;
							}
						}
						else {
							if (!verify_key ("Authenticity Root", ref manifest_dual_root.authenticity_key)) {
								return 1;
							}
						}
					}

					if (test_op (OPERATION_VERIFY_ROOT_KEY_SECONDARY)) {
						if (single_root_manifest) {
							throw new Exception ("Single root manifest does not include authority key!");
						}
						else {
							if (!verify_key ("Authority Root", ref manifest_dual_root.authority_key)) {
								return 1;
							}
						}
					}

					if (test_op (OPERATION_VERIFY_SIGNING_KEY)) {
						if (single_root_manifest) {
							if (!verify_key ("Signing", ref manifest.data.signing_key)) {
								return 1;
							}
						}
						else {
							if (!verify_key ("Signing", ref manifest_dual_root.data.signing_key)) {
								return 1;
							}
						}
					}

					if (test_op (OPERATION_VERIFY_SECONDARY_KEY)) {
						if (single_root_manifest) {
							if (manifest.header_signed.valid_keys < 2) {
								Console.WriteLine ("Manifest does not have a 2nd key set!");
								return 1;
							}

							if (!verify_key ("Secondary", ref manifest.data.secondary_key)) {
								return 1;
							}
						}
						else {
							if (manifest_dual_root.header_signed.valid_keys < 2) {
								Console.WriteLine ("Manifest does not have a 2nd key set!");
								return 1;
							}

							if (!verify_key ("Secondary", ref manifest_dual_root.data.secondary_key)) {
								return 1;
							}
						}
					}

					if (test_op (OPERATION_VERIFY_ROOT_KEY_MANIFEST)) {
						if (single_root_manifest) {
							if (!verify_header ("Key manifest Authenticity Key", ref manifest.owner_key,
								ref manifest.signature, ref manifest.header_signed,
								ref manifest.header_signed.digest, ref manifest.data)) {
								return 1;
							}
						}
						else {
							if (!verify_header ("Key manifest Authenticity Key", ref manifest_dual_root.authenticity_key,
								ref manifest_dual_root.authenticity_signature, ref manifest_dual_root.header_signed,
								ref manifest_dual_root.header_signed.digest, ref manifest_dual_root.data)) {
								return 1;
							}
						}
					}

					if (test_op (OPERATION_VERIFY_ROOT_KEY_MANIFEST_SECONDARY)) {
						if (single_root_manifest) {
							throw new Exception ("Single root manifest does not include authority key!");
						}
						else {
							if (!verify_header ("Key manifest Authority Key", ref manifest_dual_root.authority_key,
								ref manifest_dual_root.authority_signature, ref manifest_dual_root.header_signed,
								ref manifest_dual_root.header_signed.digest, ref manifest_dual_root.data)) {
								return 1;
							}
						}
					}
				}
			}

			if (test_op (OPERATION_VERIFY_FIRMWARE)) {
				Console.WriteLine ("Verifying firmware...");
				if (pkg_magic == TGM_MAGIC) {
					if (!verify_sig ("Firmware (tenant)", ref grant.data.tenant_key,
						ref firmware.signature, ref firmware.header_signed)) {
						return 1;
					}
				}
				else {
					if (single_root_manifest) {
						if (!verify_sig ("Firmware (primary)", ref manifest.data.signing_key,
							ref firmware.signature, ref firmware.header_signed)) {
							return 1;
						}

						if ((manifest.header_signed.valid_keys > 1) && !verify_sig ("Firmware (secondary)",
							ref manifest.data.secondary_key, ref firmware.secondary_signature,
							ref firmware.header_signed)) {
							return 1;
						}
					}
					else {
						if (!verify_sig ("Firmware (primary)", ref manifest_dual_root.data.signing_key,
							ref firmware.signature, ref firmware.header_signed)) {
							return 1;
						}

						if ((manifest_dual_root.header_signed.valid_keys > 1) && !verify_sig ("Firmware (secondary)",
							ref manifest_dual_root.data.secondary_key, ref firmware.secondary_signature,
							ref firmware.header_signed)) {
							return 1;
						}
					}
				}
			}

			// Update headers

			if (test_op (TENANT_OPERATION_UPDATE)) {
				Console.WriteLine ("Writing tenancy grant...");
				write_struct (writer, 0, ref grant, buffer);
			}
			else if (test_op (TRANSFER_OPERATION_UPDATE)) {
				Console.WriteLine ("Writing ownership transfer...");
				if (single_root_manifest) {
					write_struct (writer, 0, ref ownership, buffer);
				}
				else {
					write_struct (writer, 0, ref ownership_dual_root, buffer);
				}
			}

			if (test_op (MANIFEST_OPERATION_UPDATE)) {
				Console.WriteLine ("Writing key manifest...");
				if (single_root_manifest) {
					write_struct (writer, rkm_base, ref manifest, buffer);
				}
				else {
					write_struct (writer, rkm_base, ref manifest_dual_root, buffer);
				}
			}

			if (test_op (FIRMWARE_OPERATION_UPDATE)) {
				Console.WriteLine ("Writing firmware header...");
				write_struct (writer, fw_base, ref firmware, buffer);
			}
		}

		writer?.Dispose ();
	}
}

return 0;