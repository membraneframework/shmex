ExUnit.start(capture_log: true)

if not File.exists?("/dev/shm") do
  ExUnit.configure(exclude: [:shm_tmpfs, :shm_resizable])
end
