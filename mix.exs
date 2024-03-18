defmodule Shmex.Mixfile do
  use Mix.Project

  @version "0.5.1"
  @github_url "https://github.com/membraneframework/shmex"

  def project do
    [
      app: :shmex,
      version: @version,
      elixir: "~> 1.12",
      compilers: [:bundlex] ++ Mix.compilers(),
      description: "Elixir bindings for shared memory",
      package: package(),
      name: "Shmex",
      source_url: @github_url,
      docs: docs(),
      deps: deps(),
      dialyzer: dialyzer()
    ]
  end

  def application do
    [
      extra_applications: [:logger]
    ]
  end

  defp package do
    [
      maintainers: ["Membrane Team"],
      licenses: ["Apache 2.0"],
      files: ["lib", "c_src", "mix.exs", "README*", "LICENSE*", ".formatter.exs", "bundlex.exs"],
      links: %{
        "GitHub" => @github_url,
        "Membrane Framework Homepage" => "https://membraneframework.org"
      }
    ]
  end

  defp docs do
    [
      main: "readme",
      extras: ["README.md", "LICENSE"],
      formatters: ["html"],
      source_ref: "v#{@version}"
    ]
  end

  defp deps() do
    [
      {:bundlex, "~> 1.0"},
      {:bunch_native, "~> 0.5.0"},
      {:ex_doc, "~> 0.21", only: :dev, runtime: false},
      {:dialyxir, ">= 0.0.0", only: :dev, runtime: false},
      {:credo, ">= 0.0.0", only: :dev, runtime: false}
    ]
  end

  defp dialyzer() do
    opts = [
      flags: [:error_handling]
    ]

    if System.get_env("CI") == "true" do
      # Store PLTs in cacheable directory for CI
      [plt_local_path: "priv/plts", plt_core_path: "priv/plts"] ++ opts
    else
      opts
    end
  end
end
