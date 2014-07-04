# File Format Specification

This application uses a custom binary file format `.dog`. The specification for that file is detailed below.


## Version 1

V1 Files are composed of a *Header* followed by the PCM-like *Data*. The Header is made up of two segments, an *Identifier* and *Metadata* (optional).


### Header

The `.dog` file format always starts with a header like the following: 

```
FF 44 4F 47 01 07 02 01 3E FF 01 2F EE
```

Header bytes 1 through 6 of the header are the Identifier. Any remaining bytes in the header are the metadata section.

#### Identifier

```
FF 44 4F 47 01 07
```

The first byte is always `FF`, followed by the ASCII bytes that represent the string "DOG" (`44 4F 47`). These first four bytes should never change.

The next byte (`01`) indicates the file format version (1 in this case) and the last byte (`07`) indicates the length (in bytes) of the remaining data in this header (metadata). This variable length allows for optional file meta information, and possible backwards compatibility. A meta-length value of 0 is allowed, in this case there is no metadata header and the next byte in the file will begin the Data section. The Length byte must include all fields in the metadata section in its calculation.

#### Metadata

```
02 01 3E FF 01 2F EE
```

The meta-header is variable length, and optional (can be 0 length). When present, it follows a sequence of:

    [Length (bytes) of value field] [Key] [Value]

The Length field is 1 byte, and does not include the Key field in its calculation. The Key field is 1 byte and maps to which parameter is being set. The Value field is variable length, as defined by the Length field, and defines the value of the Key. There may be as many sequences as possible, that fit within the Meta section. The above Meta section may be read as:

    02: 2 bytes of value data
    01: this maps to "lossiness level"
    3E FF: set lossiness level to 16127 (0x3EFF)


    01: 1 byte of value data
    2F: some imaginary key mapped to 2F
    EE: set that variable to 238 (0xEE)


### Data

Any bytes following the header are interpreted as PCM data, with the exception of byte `00`. See 'Compression' for a description of the control command that follows `00`. 

#### Compression

`.dog` format uses a custom compression method to condense large continuous areas of similar byte ranges. Without this compression, long periods of silence (which are typical in this application's use case) would take up a significant portion of file size. This compression method aims to condense those silent periods into very few bytes.

When byte `00` is encountered in the data section, it begins a compressed data sequence. This sequence is three bytes long, including the starting `00` byte. An example sequence follows:

```
00 80 FF
```

`00` marks the start of the sequence. `80` is the data value. `FF` is the number of samples this value should repeat for. For example, an unsigned 8-bit PCM sequence of the following:

```
83 7E 81 81 81 81 81 80 83
```

could be represented in the `.dog` format as:

```
83 7E 00 81 05 80 83
```

`00` as compression start byte, `81` is the value, repeated `05` times.

Since `00` is a valid PCM data value that was changed to be a compression escape byte, it no longer represents the data value 0 anymore. To represent the value 0, you may use the following sequence: `00 00 01`. That is, `00` for escape, followed by `00` as data value, `01` times.