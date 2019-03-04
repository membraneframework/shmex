defmodule Shmex.Native do
  @moduledoc """
  This module provides natively implemented functions allowing low-level
  operations on Posix shared memory. Use with caution!
  """

  use Bundlex.Loader, nif: :shmex

  @type try_t() :: :ok | {:error, reason :: any()}
  @type try_t(result) :: {:ok, result} | {:error, reason :: any()}

  @doc """
  Creates shared memory segment and adds a guard for it.

  The guard associated with this memory segment is placed in returned
  `Shmex` struct. When the guard resource is deallocated by BEAM,
  the shared memory is unlinked and will disappear from the system when last process
  using it unmaps it
  """
  @spec allocate(shm :: Shmex.t()) :: try_t(Shmex.t())
  defnif allocate(shm)

  @doc """
  Creates guard for existing shared memory.

  This function should be only used when `Shmex` struct was created by
  some other NIF and even though the SHM exists, it's guard field is set to `nil`.
  Trying to use it with SHM obtained via `allocate/1` will result in error.

  See also docs for `allocate/1`
  """
  @spec add_guard(Shmex.t()) :: try_t(Shmex.t())
  defnif add_guard(shm)

  @doc """
  Sets the capacity of shared memory area and updates the Shmex struct accordingly.
  """
  @spec set_capacity(shm :: Shmex.t(), capacity :: pos_integer()) :: try_t(Shmex.t())
  defnif set_capacity(shm, capacity)

  @doc """
  Reads the contents of shared memory and returns it as a binary.
  """
  @spec read(shm :: Shmex.t()) :: try_t(binary())
  def read(%Shmex{size: size} = shm) do
    read(shm, size)
  end

  @doc """
  Reads `cnt` bytes from the shared memory and returns it as a binary.

  `cnt` should not be greater than `shm.size`
  """
  @spec read(shm :: Shmex.t(), cnt :: non_neg_integer()) :: try_t(binary())
  defnif read(shm, cnt)

  @doc """
  Writes the binary into the shared memory.

  Overwrites the existing content. Increases the capacity of shared memory
  to fit the data.
  """
  @spec write(shm :: Shmex.t(), data :: binary()) :: try_t(Shmex.t())
  defnif write(shm, data)

  @doc """
  Splits the contents of shared memory area into two by moving the data past
  the specified position into a new shared memory.

  `shm` has to be an existing shared memory (obtained via `allocate/1`).

  It virtually trims the existing shared memory to `position` bytes
  by setting `size` to `position` (The actual data is still present)
  and the overlapping data is copied into the new shared memory area.
  """
  @spec split_at(shm :: Shmex.t(), position :: non_neg_integer()) :: try_t({Shmex.t(), Shmex.t()})
  defnif split_at(shm, position)

  @doc """
  Concatenates two shared memory areas by copying the data from the second
  at the end of the first one.

  The first shared memory is a target that will contain data from both shared memory areas.
  Its capacity will be set to the sum of sizes of both shared memory areas.
  The second one, the source, will remain unmodified.
  """
  @spec concat(target :: Shmex.t(), source :: Shmex.t()) :: try_t(Shmex.t())
  defnif concat(target, source)

  @doc """
  Trims shared memory capacity to match its size.
  """
  @spec trim(shm :: Shmex.t()) :: try_t()
  def trim(%Shmex{size: size} = shm) do
    shm |> set_capacity(size)
  end

  @doc """
  Drops `bytes` bytes from the beggining of shared memory area and
  trims it to match the new size.
  """
  @spec trim(shm :: Shmex.t(), bytes :: non_neg_integer) :: try_t()
  def trim(shm, bytes) do
    with {:ok, trimmed_front} <- trim_leading(shm, bytes),
         {:ok, result} <- trim(trimmed_front) do
      {:ok, result}
    end
  end

  defnifp trim_leading(shm, offset)
end
