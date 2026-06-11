# Manual of **commsdsl2wireshark**

## Overview
The **commsdsl2wireshark** is a code generation tool provided by this project.
It generates [lua](https://wiki.wireshark.org/lua) dissector code for the
[wireshark](https://www.wireshark.org/).

## Command Line Arguments
The **commsdsl2wireshark** utility has multiple command line arguments, please
use `-h` option for the full list as well as default option values.

```
$> /path/to/commsdsl2wireshark -h
```
Below is a summary of most important ones.

### Selecting Schema Files
Selecting of the schema files is very similar to how it is done for the
[commsdsl2comms](Manual_commsdsl2comms.md#selecting-schema-files).

List all the schema files at the end of the command line arguments:

```
$> /path/to/commsdsl2wireshark <args> schema1.xml schema2.xml schema3.xml ...
```
The schema files will be processed **in order** of their listing.

When the input files are listed in the single file:
```
$> /path/to/commsdsl2wireshark -i schemas_list.txt
```

When a schemas listing file contains *relative* paths to the schema files use
`-p` option to specify the absolute path prefix.
```
$> /path/to/commsdsl2wireshark -i schemas_list.txt -p /path/to/schemas/dir
```

### Output Directory
By default the output CMake project is written to the current directory. It
is possible to change that using `-o` option.
```
$> /path/to/commsdsl2wireshark -o /some/output/dir schema.xml
```

### Injecting Custom Documentation
The protocol specification produced from schema file(s) is somewhat limited. It is expected
to be complemented with human readable explanation of various elements as well as conditions
under which said elements need to be used.

The **commsdsl2wireshark** utility allows injection of custom lua code snippets into the
generated project. For this
purpose `-c` option with path to directory containing custom documentation (latex code) snippets is used.
```
$> /path/to/commsdsl2wireshark -c /path/to/custom/code/dir schema.xml
```

In order to see what code injection elements are available using what files, temporarily use `--code-inject-comments`
command line option and review the generated files. The generated code will be populated with
`-- [CODE INJECT]: ` comment lines indicating places where code injection is possible.
```
$> /path/to/commsdsl2wireshark --code-inject-comments ...
```

### Choosing Default Network Port
To be able to select a default (overridable by the wireshark command like arguments) network port use
`--default-port` option.
```
$> /path/to/commsdsl2wireshark --default-port 54321 ...
```

