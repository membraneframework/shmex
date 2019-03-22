defmodule Shmex.BundlexProject do
  use Bundlex.Project

  def project do
    [
      nifs: nifs(),
      libs: libs()
    ]
  end

  defp nifs() do
    [
      shmex: [
        deps: [shmex: :lib, bunch_native: :bunch],
        sources: ["shmex.c"]
      ]
    ]
  end

  defp libs() do
    [
      lib: [
        deps: [bunch_native: :bunch],
        sources: ["lib.c"]
      ]
    ]
  end
end
