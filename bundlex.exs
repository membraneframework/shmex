defmodule Shmex.BundlexProject do
  use Bundlex.Project

  def project do
    [
      nifs: nifs(Bundlex.platform())
    ]
  end

  defp nifs(_platform) do
    [
      shmex: [
        deps: [shmex: :lib],
        sources: ["shmex.c"]
      ],
      lib: [
        export_only?: Mix.env() != :test,
        sources: ["lib.c"]
      ]
    ]
  end
end
