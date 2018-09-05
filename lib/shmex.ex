defmodule Shmex do
  @moduledoc """
  This module allows using payload placed in POSIX shared memory on POSIX
  compliant systems.

  Defines a struct representing the actual shared memory object. The struct
  should not be modified, and should always be passed around as a whole - see
  `t:#{inspect(__MODULE__)}.t/0`
  """
  alias __MODULE__.Native

  @typedoc """
  Struct describing payload kept in shared memory. Should not be modified
  and should always be passed around as a whole

  ...including passing to the native code - there are functions in `:shmex_lib`
  (a native library exported via Bundlex) that will allow to transorm Elixir
  struct into a C struct and then access the shared memory from the native code.)

  Shared memory should be available as long as the associated struct is not
  garbage collected.
  """
  @type t :: %__MODULE__{
          name: binary(),
          guard: reference(),
          size: non_neg_integer(),
          capacity: pos_integer()
        }

  defstruct name: nil, guard: nil, size: 0, capacity: 4096

  @doc """
  Creates a new, empty Shm payload with the given capacity
  """
  @spec empty(capacity :: pos_integer) :: t()
  def empty(capacity \\ 4096) do
    {:ok, payload} = create(capacity)
    payload
  end

  @doc """
  Creates a new Shm payload from existing data.
  """
  @spec new(binary()) :: t()
  def new(data) when is_binary(data) do
    new(data, byte_size(data))
  end

  @doc """
  Creates a new Shm payload initialized with `data` and set capacity.

  The actual capacity is the greater of passed capacity and data size
  """
  @spec new(data :: binary(), capacity :: pos_integer()) :: t()
  def new(data, capacity) when capacity > 0 do
    {:ok, payload} = create(capacity)
    {:ok, payload} = Native.write(payload, data)
    payload
  end

  @doc """
  Sets the capacity of SHM.

  If the capacity is smaller than the current size, data will be discarded and size modified
  """
  @spec set_capacity(t(), pos_integer()) :: t()
  def set_capacity(payload, capacity) do
    {:ok, new_payload} = Native.set_capacity(payload, capacity)
    new_payload
  end

  defp create(capacity) do
    shm_struct = %__MODULE__{capacity: capacity}
    Native.allocate(shm_struct)
  end
end
