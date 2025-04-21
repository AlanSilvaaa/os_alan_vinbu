# Truth table
Shows the Truth table for the given two numbers using assembly x64 for Linux.

## Operations
The table show the next operations:
- AND
- OR
- XOR
- NAND
- NOR

## Requirements
- `nasm` (Assembler)
- `binutils` (the GNU linker)
- a Linux system with x86_64 architecture


## Compile
To compile the code, you just need to execute the compile script:
```bash
./compile
```

## Run
To run the code, execute the `truth_table`
```bash
./truth_table
```

## Docker
You can run the code using Docker. The Dockerfile is included in the repository. To build the Docker image, run:
```bash
docker build -t truth_table .
```

To run the Docker container, use:
```bash
docker run --rm truth_table
```

