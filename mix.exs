defmodule Membrane.Common.C.Mixfile do
  use Mix.Project
  Application.put_env(:bundlex, :membrane_common_c, __ENV__)

  @version "0.2.0"

  def project do
    [
      app: :membrane_common_c,
      version: @version,
      elixir: "~> 1.6",
      elixirc_paths: elixirc_paths(Mix.env()),
      compilers: [:bundlex] ++ Mix.compilers(),
      description: "Membrane Multimedia Framework (C language common routines)",
      package: package(),
      name: "Membrane: Common C",
      source_url: link(),
      docs: docs(),
      deps: deps()
    ]
  end

  defp elixirc_paths(:test), do: ["lib", "test/support"]
  defp elixirc_paths(_), do: ["lib"]

  defp link do
    "https://github.com/membraneframework/membrane-common-c"
  end

  defp package do
    [
      maintainers: ["Membrane Team"],
      licenses: ["Apache 2.0"],
      files: ["lib", "c_src", "mix.exs", "README*", "LICENSE*", ".formatter.exs", "bundlex.exs"],
      links: %{
        "GitHub" => link(),
        "Membrane Framework Homepage" => "https://membraneframework.org"
      }
    ]
  end

  defp docs do
    [
      main: "readme",
      extras: ["README.md"],
      source_ref: "v#{@version}"
    ]
  end

  defp deps() do
    [
      {:ex_doc, "~> 0.19", only: :dev, runtime: false},
      {:bundlex, "~> 0.1.3"}
    ]
  end
end
