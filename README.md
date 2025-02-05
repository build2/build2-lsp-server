# build2-lsp-server - An LSP server for the build2 toolchain

## Status

Very early alpha. Be warned.
Far from conformant with LSP spec.

## Features

### Syntax Highlighting (Semantic Tokens)

Naive tokenization implementation for `buildfile` and `manifest`.
Regarding `buildfile`, decision regarding parsing approach so far deferred. See [discussion here](https://github.com/build2/build2/issues/109).

## Usage

Current lack of LSP conformance aside, the server should in theory be usable in any LSP-enabled context.

## Integrations

- [VS Code client extension](https://github.com/kamrann/build2-vscode)
