Coding Style
============

EditorConfig and ClangFormat
----------------------------

The `.editorconfig` and `clang-format` tools are used to enforce style guides. Ensure support for these is enabled in your editor for the smoothest experience.

File Header
-----------

Every file should contain the following copyright header:

```c++
// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

```

Spacing and Indentation
-----------------------

Indentation uses four (4) spaces. Never tabs.

Spacing around binary operators: 1 + 1

Spacing after control keywords and parenthesis: if (test)

No spacing around function name or argument: foo(bar, baz)

Braces
------

Braces are on the opening line, but never trail on closing lines:

```c++
if (test) {
    true_body;
}
else {
    false_false;
}
```

Naming
------

Namespaces are snake_case or preferably short abbreviations.

Classes are in PascalCase, also called UpperCamelCase.

Functions are in camelCase, also called lowerCamelCase.

Variables and local constants are in snake_case. Global constants are in UPPER_CASE.

