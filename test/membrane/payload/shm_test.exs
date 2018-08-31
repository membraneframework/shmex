defmodule Shmex.ProtocolImplTest do
  use ExUnit.Case

  alias Membrane.Payload
  alias Payload.Shm

  describe "Given empty shm payload" do
    setup :empty_shm

    test "Payload.size/1 should return 0", %{shm: shm} do
      assert Payload.size(shm) == 0
    end

    test "Payload.to_binary/1 return an empty binary", %{shm: shm} do
      assert Payload.to_binary(shm) == ""
    end

    test "Payload.split_at/2 should fail for any at_pos value", %{shm: shm} do
      0..3
      |> Enum.each(fn at_pos ->
        assert_raise FunctionClauseError, fn ->
          Payload.split_at(shm, at_pos)
        end
      end)
    end

    test "Payload.module/1 should return Shm module", %{shm: shm} do
      assert Payload.module(shm) == Shm
    end
  end

  describe "Given shm payload with content" do
    setup :example_shm

    test "Payload.size/1 should return byte_size of content", ctx do
      %{shm: shm, content: content} = ctx
      assert Payload.size(shm) == byte_size(content)
    end

    test "Payload.to_binary/1 should return the content", ctx do
      %{shm: shm, content: content} = ctx
      assert Payload.to_binary(shm) == content
    end

    test "Payload.split_at/2 should fail when at_pos is 0", %{shm: shm} do
      assert_raise FunctionClauseError, fn ->
        Payload.split_at(shm, 0)
      end
    end

    test "Payload.split_at/2 should fail when at_pos is greater or equal to size of content",
         ctx do
      %{shm: shm, content: content} = ctx

      assert_raise FunctionClauseError, fn ->
        Payload.split_at(shm, byte_size(content))
      end

      assert_raise FunctionClauseError, fn ->
        Payload.split_at(shm, byte_size(content) + 15)
      end
    end

    test "Payload.split_at/2 should create 2 payloads for valid at_pos", ctx do
      %{shm: shm, content: content} = ctx
      at_pos = 10
      assert {shm_head, shm_tail} = Payload.split_at(shm, at_pos)
      <<content_hd::binary-size(at_pos), content_tail::binary>> = content
      assert Payload.to_binary(shm_head) == content_hd
      assert Payload.to_binary(shm_tail) == content_tail
    end

    test "Payload.module/1 should return shm module", %{shm: shm} do
      assert Payload.module(shm) == Shm
    end
  end

  def example_shm(_) do
    content = "1235456789abcdefgh"

    [
      shm: Shm.new(content),
      content: content
    ]
  end

  def empty_shm(_) do
    [
      shm: Shm.empty()
    ]
  end
end
