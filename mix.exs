defmodule Shmex.Mixfile do
  use Mix.Project

  @version "0.2.1"
  @github_url "https://github.com/membraneframework/shmex"

  def project do
    [
      app: :shmex,
      version: @version,
      elixir: "~> 1.7",
      compilers: [:bundlex] ++ Mix.compilers(),
      description: "Elixir bindings for shared memory",
      package: package(),
      name: "Shmex",
      source_url: @github_url,
      docs: docs(),
      deps: deps()
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
      extras: ["README.md"],
      source_ref: "v#{@version}"
    ]
  end

  defp deps() do
    [
      {:ex_doc, "~> 0.21", only: :dev, runtime: false},
      # {:bundlex, "~> 0.2.8"},
      {:bundlex, git: "git@github.com:membraneframework/bundlex.git"},
      # {:bunch_native, "~> 0.2.0"}
      {:bunch_native, git: "git@github.com:membraneframework/bunch-native.git", branch: "bundlex"}
    ]
  end
end
