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
        deps: [shmex: :lib, bunch_native: :bunch],
        sources: ["shmex.c"]
      ],
      lib: [
        deps: [bunch_native: :bunch],
        export_only?: Mix.env() != :test,
        sources: ["lib.c"]
      ]
    ]
  end
end
