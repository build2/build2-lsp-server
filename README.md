# build2-lsp-server - An LSP server for the build2 toolchain

## Status

Very early alpha. Be warned.
Far from conformant with LSP spec.

## Building

The project is written using C++20 modules. It should compile on recent versions of both MSVC and Clang (tested on LLVM 19+ with libc++), without needing any special configuration.

### Configuration variables

This package provides the following configuration variables:

```
[bool] config.build2_lsp_server.enable_import_std
```

`enable_import_std`: Toggles use of `import std`. Currently enabled by default for MSVC.

## Implementation Approach

Package is dependent on and being developed in tandem with the [lsp-boot](https://github.com/kamrann/lsp-boot) library, which provides some infrastructure around the LSP and its message types. This may yet be discarded if a satisfactory existing library is found. For the moment the approach is therefore to largely work directly with `boost.json` values when manipulating message structures; long term, if the library is maintained, the intention would be to codegen static types from the LSP specification machine-readable form.

## Features

### Syntax Highlighting (Semantic Tokens)

Naive tokenization implementation for `buildfile` and `manifest`.
Regarding `buildfile`, decision regarding parsing approach so far deferred. See [discussion here](https://github.com/build2/build2/issues/109).

## Usage

Current lack of LSP conformance aside, the server should in theory be usable in any LSP-enabled context.

## Integrations

- [VS Code client extension](https://github.com/build2/build2-vscode)
