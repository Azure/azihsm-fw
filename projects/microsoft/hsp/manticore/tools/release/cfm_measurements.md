
# CFM Measurements

List of measurements that are included in generating CFM xml from the Manticore Firmware binary file.

## Description
* Measurement ID: Unique identifier for each measurement within the CFM according to the firmware
* Image Component: Specifies the firmware image component from which the data is extracted. It could be:
    * NA: Not applicable
    * comp: An array of tuples for each firmware image component
    * root: The root key manifest on the boot image.
    * boot: The 1SP boot image.
    * keys: The firmware key manifest.
    * desc: The firmware descriptor.

* Event ID: Identifier for the event associated with the measurement in firmware.
* Version: Version number associated with the measurement in firmware.
* Offset: The offset in bytes within the firmware image where the measurement data is located.
* Length: Length of the measurement in bytes

| Name                       | Measurement ID | Image Component             | Event ID  | Version | Offset | Length | Comment/description                                          |
|----------------------------|----------------|-----------------------------|-----------|---------|--------|--------|--------------------------------------------------------------|
| SPRTFirmwareBuildVersion   | 15             | desc                        | e1000105  | 0       | 23     | 8      | Build Version parsed from the firmware descriptor            |
| DeviceState                | 1              | Static value                | e1000000  | 0       | NA     | NA     | static value where last 4 bytes are masked out               |
| ROMOwnerPublicKey          | 2              | root                        |  e1000001 | 0       | 4      | 96     | owner public key in root key manifest                        |
| ROMKeyManifestSVN          | 3              | root                        | e1000002  | 0       | 200    | 4      | MANTICORE_1SP_SVN parsed from key manifest header            |
| ROMFirmwarePublicKey       | 5              | root                        | e1000004  | 0       | 256    | 96     | FW public key in root key manifest                           |
| ROMSecondaryPublicKey      | 6              | root                        | e1000005  | 0       | 352    | 96     | Secondary Public key from root key manifest                  |
| ROMFirmwareSVN             | 7              | root                        | e1000007  | 0       | 200    | 4      | 1SP_SVN parsed from 1SP image                                |
| ROMFirmwareBuildVersion    | 8              | boot                        | e1000008  | 0       | 220    | 8      | MANTICORE_1SP_BUILD_VERSION parsed from the 1SP image itself |
| FirmwareImage              | 9              | boot                        | e1000009  | 0       | 236    | 48     | Digest in the 1SP image header                               |
| UnlockPolicy               | 10             | Static value                | e1000100  | 0       | NA     | NA     | NA                                                           |
| SPRTirmwareKeyManifest     | 11             | keys                        | e1000101  | 0       | 224    | 48     | Hash in the Firmware Key Manifest                            |
| SPRTFirmwareKeyManifestSVN | 12             | keys                        | e1000102  | 0       | 212    | 8      | SVN parsed from the FW key manifest header                   |
| SPRTFirmwarePublicKey      | 13             | keys                        | e1000103  | 0       | 272    | 120    | Application Public Key from Firmware Key Manifest            |
| SPRTFirmwareSVN            | 14             | Desc                        | e1000104  | 0       | 31     | 8      | MANTICORE_FW_SVN parsed from the firmware descriptor         |
| SPRTImage                  | 16             | comp [0]                    | e1000106  | 0       | NA     | NA     | Entire SPRT image                                            |
| AEBStatus                  | 17             | Static value                | e1000107  | 0       | NA     | NA     | NA                                                           |
| AEBLockedStatus            | 18             | Static value                | e1000108  | 0       | NA     | NA     | NA                                                           |
| SOCFirmwareKeyManifest     | 19             | keys                        | e1000200  | 0       | 224    | 48     | Hash in the Firmware Key Manifest                            |
| SOCFirmwareKeyManifestSVN  | 20             | keys                        | e1000201  | 0       | 212    | 8      | SVN parsed from the FW key manifest header                   |
| SOCFirmwarePublicKey       | 21             | keys                        | e1000202  | 0       | 272    | 120    | Application Public Key from Firmware Key Manifest            |
| SOCFirmwareSVN             | 22             | Desc                        | e1000203  | 0       | 31     | 8      | SVN parsed from the firmware descriptor                      |
| SOCFirmwareBuildVersion    | 23             | Desc                        | e1000204  | 0       | 23     | 8      | Build Version parsed from the firmware descriptor            |
| CPImage                    | 24             | image                       | e1000205  | 0       | NA     | NA     | The hash of the concat of hash of the sub-image(s)             |
| FP0Image                   | 25             | image                       | e1000206  | 0       | NA     | NA     | The hash of the concat of hash of the sub-image(s)             |
| FP1Image                   | 26             | image                       | e1000207  | 0       | NA     | NA     | The hash of the concat of hash of the sub-image(s)             |
| FP2Image                   | 27             | image                       | e1000208  | 0       | NA     | NA     | The hash of the concat of hash of the sub-image(s)             |
| PHYImage                   | 28             | NA                          | e1000209  | 0       | NA     | NA     | NA                                                           |
| IntrusionStatus            | 36             | Static value                | e0000031  | 0       | NA     | NA     | NA                                                           |
