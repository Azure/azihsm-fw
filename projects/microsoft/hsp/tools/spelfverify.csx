/*
 *      Copyright (C) Microsoft Corporation. All rights reserved.
 *
 * Module Name
 *
 *      spelfverify.csx
 *
 * Abstract
 *
 *      This script is loaded by spsign tool to verify signed elf binary
 *
 * Author
 *
 *      Navin Pai (navinp)
 *
 */
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;

using SpSign;

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
class VerifyElf
{
    //
    // The security version of the ELF. It can be changed on command line
    // by specifying value for elfSecVer. Default value is 1
    // It should only be incremented if the previous version had security
    // issues and needs to be revoked
    //
    const int ELF_SECURITY_VERSION_CURRENT = ${elfSecVer:1};

    public static byte[] SpSignEccAuthorityKey = new byte[]
    {
        ${ECC_AUTHORITY_KEY:0}
    };

    public static byte[] SpSignEccAuthenticityKey = new byte[]
    {
        ${ECC_AUTHENTICITY_KEY:0}
    };
    const int DER_ECDSA_P384_SIGNATURE_MAX_SIZE		= (8 + SpUtil.Ecc384SignatureSize);

    /// <summary>
    /// This struct matches the section definition in the hspelf.h
    /// </summary>
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    struct HSP_ELF_SECTION_SIGNATURE
    {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = DER_ECDSA_P384_SIGNATURE_MAX_SIZE)]
        public byte[] EccAuthenticity;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = DER_ECDSA_P384_SIGNATURE_MAX_SIZE)]        
        public byte[] EccAuthority;

        public struct ELF_SIGNED
        {
            public uint SecurityVersion;

            [MarshalAs(UnmanagedType.ByValArray, SizeConst = SpUtil.SpMsg384Size)]
            public byte[] ElfPlaintextHash;
        }

        public ELF_SIGNED Signed;
    }

    static bool decode_der (byte[] der_signature, ref byte[] sig_bytes)
    {
        byte[] sig_R = new byte[(SpUtil.Ecc384SignatureSize/2)];
        byte[] sig_S = new byte[(SpUtil.Ecc384SignatureSize/2)];

        Console.WriteLine ("Parsing DER to ECC signature...");
        var decoder = new der_decoder (der_signature);

        if (!decoder.verify_tag (0x30)) {
            return false;
        }

        if (!decoder.extract_big_uint (sig_R)) {
            return false;
        }

        if (!decoder.extract_big_uint (sig_S)) {
            return false;
        }
        Array.Copy(sig_R, 0, sig_bytes, 0, SpUtil.Ecc384SignatureSize/2);
        Array.Copy(sig_S, 0, sig_bytes, SpUtil.Ecc384SignatureSize/2, SpUtil.Ecc384SignatureSize/2);

        return true;      
    }


    public static int Verify()
    {
        int sizeofHdr = Marshal.SizeOf(typeof(HSP_ELF_SECTION_SIGNATURE));
        int sizeofSigned = Marshal.SizeOf(typeof(HSP_ELF_SECTION_SIGNATURE.ELF_SIGNED));

        Console.WriteLine(@"Reading input elf file : ${inputFile}");

        ElfFile elfFile = new ElfFile(@"${inputFile}");

        //
        // Get the signed data and construct the object from data
        //
        byte[] signedBlob = elfFile.GetSectionData(".hspSignature");
        HSP_ELF_SECTION_SIGNATURE elfSignature = (HSP_ELF_SECTION_SIGNATURE)
            SpUtil.GetStructFromBytes(typeof(HSP_ELF_SECTION_SIGNATURE), signedBlob);

        //
        // Compute the hash of the file and make sure that the hash matches
        //

        Console.WriteLine("Computing plaintext hash...");
        byte[] signedContent = elfFile.GetSigningContent(".hspSignature");

        byte[] plainTextHash = SpUtil.ComputeHash(HashAlgo.Sha384, signedContent);
        if (!plainTextHash.SequenceEqual(elfSignature.Signed.ElfPlaintextHash))
        {
            throw new Exception("Plaintext hash does not match");
        }

        //
        // Compute the header digest
        //

        Console.WriteLine("Compute hash of header...");
        byte[] elfSigned = SpUtil.GetBytesFromStruct(elfSignature.Signed);
        byte[] hdrEccHash = SpUtil.ComputeHash(HashAlgo.Sha384, elfSigned);

        byte[] sigBytes = new byte[SpUtil.Ecc384SignatureSize];

        //
        // Verify the hash and signature using public key
        //
        if(SpSignEccAuthorityKey.Length > 1)
        {        
            decode_der(elfSignature.EccAuthority, ref sigBytes);

            Console.WriteLine("Verifying ECC Signature using Authority keys...");
            if (!SpUtil.VerifyHash(KeyProvider.EccPublicBlob, SpSignEccAuthorityKey,
                HashAlgo.Sha384, hdrEccHash, sigBytes, false))
            {
                Console.ForegroundColor = ConsoleColor.Red;
                throw new Exception("ECC Verification failed!");
            }
        }

        Array.Clear(sigBytes, 0, sigBytes.Length);
        decode_der(elfSignature.EccAuthenticity, ref sigBytes);

        Console.WriteLine("Verifying ECC Signature using Authenticity keys...");
        if (!SpUtil.VerifyHash(KeyProvider.EccPublicBlob, SpSignEccAuthenticityKey,
            HashAlgo.Sha384, hdrEccHash, sigBytes, false))
        {
            Console.ForegroundColor = ConsoleColor.Red;
            throw new Exception("ECC Verification failed!");
        }    

        Console.ForegroundColor = ConsoleColor.Green;
        Console.WriteLine("Verification succeeded!");
        Console.ResetColor();
        
        return 0;
    }
}

return VerifyElf.Verify();
