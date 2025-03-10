When working with **C89**  (ANSI C, aka this project's n64 'C' code), it's best to follow a **clear, consistent, and readable**  naming convention that avoids modern C features (like `bool`) while still maintaining maintainability. Here’s a structured approach:
---

Quick reference:
`SCREAMING_SNAKE_CASE`
MACROS_WORD
CONSTANTS_WORD
ENUM_MEMBERS

`prefix_PascalCase`
file_function()
gGlobal
sStatic
vVolatile
pPointer
aArray


---

**General Naming Guidelines**  
1. **Use camelCase for functions and variables**  
  - `doSomething()`, `calculateResult()`, `bufferSize`
 
2. **Use uppercase with underscores for macros and constants**  
  - `#define MAX_BUFFER_SIZE 1024`
 
  - `const int DEFAULT_TIMEOUT = 1000;`
 
3. **Use `CamelCase` with `t_` prefix for `typedef` structs**  
  - `typedef struct t_person { ... } s_person;`
 
  - Or `typedef struct t_fileHandler { ... } s_fileHandler;`
 
4. **Use `enumName_e` for enums and `SCREAMING_CASE` for enum members** 
```c
typedef enum t_color {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
} e_color;
```

5. **Typedef structs are CamelCase_s, and members are camelCase**
```c
typedef struct{
  value = int;
  example = u32;
} StructName_s

```
 
5. **Prefix global variables to indicate scope**  
  - Use `g` for global variables: `gLogLevel`
 
  - Use `s` for static variables: `static int sCounter;`
 
6. **Use `p` for pointers and `a` for arrays**  
  - `char *pName;`
 
  - `int aValues[10];`
 
---

**Examples of Naming in Practice** **Functions** 

```c
int calculateChecksum(const char *data);
void processData(void);
void handleError(int error_code);
```
**Variables** 
```c
int gTotalCount;  // Global variable
static int sLocalCache;  // Static variable
char *pBuffer;  // Pointer
int aNumbers[10];  // Array
```

**Structs** 
```c
typedef struct {
    char aName[50];
    int age;
} Person_s;
```

**Enums** 

```c
typedef enum {
    ERROR_NONE,
    ERROR_INVALID_INPUT,
    ERROR_OUT_OF_MEMORY
} ErrorCode_e;
```

---

This is a **large project with modular code** , consider using **module-based prefixes** :

```c
// File: network.c
int net_init();  
int net_sendData(const char *data);

// File: file_io.c
int file_open(const char *path);
void file_closeFile();
```

This makes it clear where a function belongs. Filenames must also be lowercase/snake_case, to allow for
case-insensitive operating systems. Only prefix filenames, not directory names.

