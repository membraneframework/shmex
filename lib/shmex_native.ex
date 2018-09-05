defmodule Shmex.Native do
  @moduledoc """
  This module provides natively implemented functions allowing low-level
  operations on Posix shared memory. Use with caution!
  """

  alias Bunch.Type
  use Bundlex.Loader, nif: :shmex

  @doc """
  Creates shared memory segment and adds a guard for it.

  The guard associated with this memory segment is placed in returned
  `Shmex` struct. When the guard resource is deallocated by BEAM,
  the shared memory is unlinked and will disappear from the system when last process
  using it unmaps it
  """
  @spec allocate(payload :: Shmex.t()) :: Type.try_t(Shmex.t())
  defnif allocate(payload)

  @doc """
  Creates guard for existing shared memory.

  This function should be only used when `Shmex` struct was created by
  some other NIF and even though the SHM exists, it's guard field is set to `nil`.
  Trying to use it with SHM obtained via `create/1` will result in error.

  See also docs for `create/1`
  """
  @spec add_guard(Shmex.t()) :: Type.try_t(Shmex.t())
  defnif add_guard(payload)

  @doc """
  Sets the capacity of SHM and updates the struct accordingly
  """
  @spec set_capacity(payload :: Shmex.t(), capacity :: pos_integer()) :: Type.try_t(Shmex.t())
  defnif set_capacity(payload, capacity)

  @doc """
  Reads the contents of SHM and returns as binary
  """
  @spec read(payload :: Shmex.t()) :: Type.try_t(binary())
  def read(%Shmex{size: size} = payload) do
    read(payload, size)
  end

  @doc """
  Reads `cnt` bytes from SHM and returns as binary

  `cnt` should not be greater than `payload.size`
  """
  @spec read(payload :: Shmex.t(), cnt :: non_neg_integer()) :: Type.try_t(binary())
  defnif read(payload, cnt)

  @doc """
  Writes the binary into the SHM.

  Overwrites existing content. Increases capacity to fit the data.
  """
  @spec write(payload :: Shmex.t(), data :: binary()) :: Type.try_t(Shmex.t())
  defnif write(payload, data)

  @doc """
  Splits the contents of SHM into 2 by moving part of the data into a new SHM

  `payload` has to be an existing shm (obtained via `allocate/1`).

  It virtually trims the existing SHM to `position` bytes by setting `size` to `position`
  (The actual data is still present) and the overlapping data is copied into the new SHM.
  """
  @spec split_at(payload :: Shmex.t(), position :: non_neg_integer()) ::
          Type.try_t({Shmex.t(), Shmex.t()})
  defnif split_at(payload, position)

  @spec concat(left :: Shmex.t(), right :: Shmex.t()) :: Type.try_t(Shmex.t())
  defnif concat(left, right)

  @spec trim(payload :: Shmex.t()) :: Type.try_t()
  def trim(%Shmex{size: size} = shm) do
    shm |> set_capacity(size)
  end

  @spec trim(payload :: Shmex.t(), bytes :: non_neg_integer) :: Type.try_t()
  def trim(payload, bytes) do
    with {:ok, trimmed_front} <- trim_leading(payload, bytes),
         {:ok, result} <- trim(trimmed_front) do
      {:ok, result}
    end
  end

  defnifp trim_leading(payload, offset)
end
