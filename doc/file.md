# File Format Specification

This application uses a custom binary file format `.dog`. The specification for that file is detailed below.


## Version 1

### Header

The `.dog` file format always starts with a header like the following: 

```
FF 44 4F 47 01 00
```

The first byte is always `FF`, followed by the ASCII bytes that represent the string "DOG" (`44 4F 47`). The next byte `01` indicates the file format version (1 in this case) and the last byte `00` indicates the "lossiness" used in compression. 

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