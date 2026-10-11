# libpnm

C library to load and save PNM images in plain (ASCII) format:

| Format | Magic number | Extension | Content |
|--------|--------------|-----------|---------|
| PBM    | `P1`         | `.pbm`    | black and white (0 or 1) |
| PGM    | `P2`         | `.pgm`    | grayscale (0 to `max_val`) |
| PPM    | `P3`         | `.ppm`    | color, 3 values (R G B) per pixel |

INFO0030 — Project 1.

## Build

```sh
make          # builds libpnm.a and the pnm executable
make debug    # rebuilds with -g and the address and undefined sanitizers
make clean    # removes generated files
```

Compiler flags: `-std=c99 -pedantic -Wall -Wextra`.

## Usage

```sh
./pnm -f <PBM|PGM|PPM> <file_path>
./pnm -h
```

| Argument  | Role |
|-----------|------|
| `argv[1]` | the `-f` option (or `-h` for help) |
| `argv[2]` | the format: `PBM`, `PGM` or `PPM` (upper or lower case) |
| `argv[3]` | the path of the image to load |

The program loads the image, then writes it back to `output.<ext>` (`output.ppm`, `output.pgm` or `output.pbm`) in the current directory. An existing `output.<ext>` file is overwritten.

Example (the program's messages are in French):

```sh
$ ./pnm -f PPM exemples_images/escargot.ppm
Image exemples_images/escargot.ppm chargée (PPM)
Image écrite dans output.ppm
```

The program exits with a non-zero status if:

- `-f` is missing, has no argument, or an option is unknown;
- the format is not PBM, PGM or PPM;
- the format does not match the file extension;
- the file path is missing, or there are too many arguments;
- loading or writing the image fails.

Options must come before the file path (`./pnm -f PPM image.ppm`).

## API

Declared in `pnm.h`. The `PNM` type is opaque.

```c
int  load_pnm(PNM **image, char *filename);
int  write_pnm(PNM *image, char *filename);
void free_pnm(PNM *image);
```

### `load_pnm`

Loads the image `filename` and stores its address in `*image`.

| Return | Meaning |
|--------|---------|
| `0`    | success |
| `-1`   | memory allocation error |
| `-2`   | malformed file name (invalid extension, file not found) |
| `-3`   | malformed file content |

The content is considered malformed if: the magic number is not `P1`, `P2` or `P3`, the width or height is 0, the max value is outside 1 to 65535 (PGM and PPM), or pixels are missing. Comments (`# ...`) are skipped. On failure, `*image` is `NULL`.

### `write_pnm`

Writes `image` to `filename`. No line in the written file is longer than 70 characters, as the format requires. The image is not freed.

| Return | Meaning |
|--------|---------|
| `0`    | success |
| `-1`   | malformed file name (extension other than `pbm`, `pgm` or `ppm`) |
| `-2`   | error opening the file |

### `free_pnm`

Frees an image loaded by `load_pnm`. Accepts `NULL`.

### Example

```c
#include "pnm.h"

PNM *image = NULL;
if (load_pnm(&image, "exemples_images/monalisa.pgm") == 0) {
    write_pnm(image, "copy.pgm");
    free_pnm(image);
}
```

## Files

| File                 | Content |
|----------------------|---------|
| `pnm.h`, `pnm.c`     | `PNM` type, loading, writing, freeing |
| `utils.h`, `utils.c` | file name extension, skipping whitespace and comments |
| `main.c`             | `pnm` program (option parsing with `getopt`) |
| `exemples_images/`   | test images |

## Known limitations

- Pixel values are not checked against `max_val`, and for PBM nothing checks that they are 0 or 1.
- The image size computation (`width × height × channels`) is not protected against integer overflow: a file with huge dimensions can cause a write past the allocated buffer. Do not open images from unknown sources.
- In PBM, bits written without spaces (`0101`) are read as a single number.
- Only plain formats (`P1` to `P3`) are supported, not the binary ones (`P4` to `P6`).
