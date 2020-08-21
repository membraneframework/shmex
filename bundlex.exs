defmodule Shmex.BundlexProject do
  use Bundlex.Project

  def project do
    [
      natives: natives(),
      libs: libs()
    ]
  end

  defp natives() do
    [
      shmex: [
        interface: :nif,
        deps: [shmex: :lib_iface, bunch_native: :bunch_iface],
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
      lib_iface: [
        interface: :nif,
        deps: [shmex: :lib, bunch_native: :bunch_iface],
        src_base: "shmex/lib_nif/shmex",
        sources: ["lib_nif.c"]
      ],
      lib_iface: [
        interface: :cnode,
        deps: [shmex: :lib, bunch_native: :bunch],
        src_base: "shmex/lib_cnode/shmex",
        sources: ["lib_cnode.c"]
      ]
    ]
  end
end
