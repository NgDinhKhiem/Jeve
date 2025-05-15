[![CMake Build and Release](https://github.com/NgDinhKhiem/Jeve/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/NgDinhKhiem/Jeve/actions/workflows/cmake-multi-platform.yml)
# Jeve

Jeve is a lightweight, educational scripting language and interpreter written in modern C++. It was developed as part of an undergraduate course project and is designed to be easy to read, extend, and experiment with. Jeve supports variables, arrays, user-defined functions, control flow, and basic type annotations.

## Features

- **Variables**: Integers, floats, strings, booleans, arrays.
- **Arithmetic & Logic**: Standard operators (`+`, `-`, `*`, `/`, `%`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `&&`, `||`, `!`).
- **Control Flow**: `if`/`else`, `while`, `for` loops.
- **Functions**: User-defined functions with parameters and return values.
- **Arrays**: Dynamic arrays with assignment, indexing, and built-in `insert`/`delete`.
- **Type Annotations**: Optional type hints for variables.
- **Input/Output**: `print()` and `input()` built-ins.
- **Memory Management**: Custom garbage collector with tunable heap size.
- **Error Handling**: Basic runtime error messages.

## Getting Started

### Build

Jeve uses CMake (C++17 required):

```sh
git clone <your-repo-url>
cd Jeve
cmake -B build .
cmake --build build -j4
```

This will produce the `jeve` executable in the `build/` directory.

### Usage

```sh
./build/jeve [options] <file>
```

**Options:**
- `-Xms<size>`  Set initial heap size (e.g., `-Xms1m` for 1MB)
- `-Xmx<size>`  Set maximum heap size (e.g., `-Xmx64m` for 64MB)
- `-h, --help`  Show help

### Example

Create a file `hello.jeve`:

```jeve
// Hello World in Jeve
message = "Hello, World!";
print(message);

// Arithmetic
a = 10;
b = 5;
sum = a + b;
print("Sum: " + sum);

// If/Else
if (sum > 10) {
    print("Sum is greater than 10");
} else {
    print("Sum is less than or equal to 10");
}

// While loop
i = 0;
while (i < 5) {
    print("Count: " + i);
    i = i + 1;
}
```

Run it:

```sh
./build/jeve examples/hello.jeve
```

### More Features

See the [`examples/`](examples/) directory for more scripts, including:

- Arrays: `minimal_array.jeve`, `single_array_test.jeve`
- Type annotations and input: `features.jeve`
- Loops and conditionals: `loops.jeve`
- Error handling: `error.jeve`

#### Example: Arrays and Functions

```jeve
arr = [1, 2, 3];
insert(arr, 1, 99);   // arr = [1, 99, 2, 3]
delete(arr, 2);       // arr = [1, 99, 3]

function add(a, b) {
    return a + b;
}
result = add(10, 20);
print("add(10, 20) = " + result);
```

## Project Structure

- `src/` — Interpreter source code
  - `main.cpp` — CLI entry point
  - `interpreter/` — Core interpreter, garbage collector, symbol table
  - `interpreter/ast/` — AST node definitions (expressions, statements, control flow, etc.)
- `examples/` — Example Jeve scripts
- `tests/` — (Optional) Test scripts and utilities

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

## Acknowledgements

Created by Nguyễn Dĩnh Khiêm for educational purposes. Contributions and suggestions are welcome!
