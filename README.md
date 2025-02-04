# build2-lsp-server - An executable

The `build2-lsp-server` executable is a <SUMMARY-OF-FUNCTIONALITY>.

Note that the `build2-lsp-server` executable in this package provides `build2` metadata.


## Usage

To start using `build2-lsp-server` in your project, add the following build-time
`depends` value to your `manifest`, adjusting the version constraint as
appropriate:

```
depends: * build2-lsp-server ^<VERSION>
```

Then import the executable in your `buildfile`:

```
import! [metadata] <TARGET> = build2-lsp-server%exe{<TARGET>}
```


## Importable targets

This package provides the following importable targets:

```
exe{<TARGET>}
```

<DESCRIPTION-OF-IMPORTABLE-TARGETS>


## Configuration variables

This package provides the following configuration variables:

```
[bool] config.build2_lsp_server.<VARIABLE> ?= false
```

<DESCRIPTION-OF-CONFIG-VARIABLES>
