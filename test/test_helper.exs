ExUnit.start()

if not File.exists?("/dev/shm") do
  ExUnit.configure(exclude: :shm_tmpfs)
end
