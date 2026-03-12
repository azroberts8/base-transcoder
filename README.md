# Transcoder

A basic base-N to base-N CLI tool.

## Compliling

```bash
make build
```

## Usage

```
./transcode [-s src_alphabet] [-d dst_alphabet] <input> <output>
```
Performs change of base operation on input file and stores it in output file. 

**src_alphabet:** File path of input encoding alphabet (default: bytes)  
**dst_alphabet:** File path of output encoding alphabet (default: bytes)  
**input:** Input file path  
**output:** Output file path  

## Notes

- Input file and outout file are interpreted and stored using specified src_alphabet and dst_alphabet respectively.
- If either alphabet is unspecified, standard base-265 bytes are used.
- Alphabet files specify symbols newline separated (1 symbol per line).
- Symbols may be multiple ASCII characters long.