# Shmex

[![Build Status](https://travis-ci.com/membraneframework/shmex.svg?branch=master)](https://travis-ci.com/membraneframework/shmex)

Shmex is a library providing elixir bindings for shared memory and native
functions to manipulate it in NIFs.

Documentation is available at [HexDocs](https://hexdocs.pm/shmex)

The source code is available at [GitHub](https://github.com/membraneframework/shmex)

## Installation

Add the following line to your `deps` in `mix.exs`. Run `mix deps.get`.

```elixir
{:shmex, "~> 0.1"}
```

All native stuff is exported as `:lib` [Bundlex](https://hex.pm/packages/bundlex) dependency.
To import, add the following line to your nif specification in `bundlex.exs`
```elixir
deps: [shmex: :lib]
```
and another one in your native header file
```c
#import <shmex/lib.h>
```

## Copyright and License

Copyright 2018, [Software Mansion](https://swmansion.com/?utm_source=git&utm_medium=readme&utm_campaign=membrane)

[![Software Mansion](https://membraneframework.github.io/static/logo/swm_logo_readme.png)](https://swmansion.com/?utm_source=git&utm_medium=readme&utm_campaign=membrane)

Licensed under the [Apache License, Version 2.0](LICENSE)
