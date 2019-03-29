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
        src_base: "shmex/nif/shmex",
        sources: ["lib.c", "../../common/lib.c"]
      ],
      lib_cnode: [
        src_base: "shmex/cnode/shmex",
        sources: ["lib.c", "../../common/lib.c"]
      ]
    ]
  end
end
