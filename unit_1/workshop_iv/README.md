# OS_Taller_4
Images Conversion to format and gray scale

## Compile
create a build directory and run cmake
```bash
mkdir build
cd build
cmake ..
make -j
```

## Usage
### Convert to format
Example to convert all images in the `images` directory to `png` format and save them in the `out` directory using 6 threads:
```bash
./cvutils -f -i "../images" -o "../out" -n 6 -t "png"
```

### Convert to gray scale
Example to convert all images in the `images` directory to gray scale and save them in the `out` directory using 6 threads:
```bash
./cvutils -g -i "../images" -o "../out" -n 6
```
