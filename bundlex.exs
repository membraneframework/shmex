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
        deps: [shmex: :lib_nif, bunch_native: :bunch_nif],
        sources: ["shmex.c"]
      ]
    ]
  end

  defp libs() do
    [
      lib: [
        deps: [bunch_native: :bunch],
        src_base: "shmex/lib/shmex",
        sources: ["lib.c"],
        libs: if(Bundlex.platform() == :linux, do: ["rt"], else: [])
      ],
      lib_nif: [
        deps: [shmex: :lib, bunch_native: :bunch_nif],
        src_base: "shmex/lib_nif/shmex",
        sources: ["lib_nif.c"]
      ],
      lib_cnode: [
        deps: [shmex: :lib, bunch_native: :bunch],
        src_base: "shmex/lib_cnode/shmex",
        sources: ["lib_cnode.c"]
      ]
    ]
  end
end
