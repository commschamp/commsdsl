# Manual of **commsdsl2latex**

## Overview
The **commsdsl2latex** is a code generation tool provided by this project.
It generates CMake project with [LaTeX](https://en.wikipedia.org/wiki/LaTeX) files
that can be used to produce protocol specification in **pdf** and/or **html** formats.


## Command Line Arguments
The **commsdsl2latex** utility has multiple command line arguments, please
use `-h` option for the full list as well as default option values.

```
$> /path/to/commsdsl2latex -h
```
Below is a summary of most important ones.

### Selecting Schema Files
Selecting of the schema files is very similar to how it is done for the
[commsdsl2comms](Manual_commsdsl2comms.md#selecting-schema-files).

List all the schema files at the end of the command line arguments:

```
$> /path/to/commsdsl2latex <args> schema1.xml schema2.xml schema3.xml ...
```
The schema files will be processed **in order** of their listing.

When the input files are listed in the single file:
```
$> /path/to/commsdsl2latex -i schemas_list.txt
```

When a schemas listing file contains *relative* paths to the schema files use
`-p` option to specify the absolute path prefix.
```
$> /path/to/commsdsl2latex -i schemas_list.txt -p /path/to/schemas/dir
```

### Output Directory
By default the output CMake project is written to the current directory. It
is possible to change that using `-o` option.
```
$> /path/to/commsdsl2latex -o /some/output/dir schema.xml
```

### Injecting Custom Documentation
The protocol specification produced from schema file(s) is somewhat limited. It is expected
to be complemented with human readable explanation of various elements as well as conditions
under which said elements need to be used.

The **commsdsl2latex** utility allows injection of custom LaTeX snippets into the
generated project. For this
purpose `-c` option with path to directory containing custom documentation (latex code) snippets is used.
```
$> /path/to/commsdsl2latex -c /path/to/custom/doc/elements schema.xml
```
In order to see what code injection elements are available using what files, use `--code-inject-comments`
command line option and review the generated files.
```
$> /path/to/commsdsl2latex --code-inject-comments ...
```
The generated files will contain multiple comment lines starting with `% [CODE INJECT]: ` prefix.
For example:
```
% [CODE INJECT]: Replace packages definition with "test1_doc.tex.package".
\usepackage[T1]{fontenc}
\usepackage[colorlinks]{hyperref}
\usepackage{nameref}
\usepackage{array}
\usepackage{booktabs}
\usepackage{longtable}

\setlength\LTleft{15pt}
\setlength\LTright{15pt}

% [CODE INJECT]: Append packages definition with "test1_doc.tex.package_append".

...

% [CODE INJECT]: Replace title (whole section) with "test1_doc.tex.title".
\title{Protocol "test1"}
\date{\today}
% [CODE INJECT]: Append to title info with "test1_doc.tex.title_append".
```

