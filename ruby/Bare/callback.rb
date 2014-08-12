require '../cpx.rb'
require './config.rb'

#trigger on button press A15 or return after 30 seconds, use 0 instead of 30000 for infinite wait.

c = CPX.new(@host,@user,@deviceid)
commands = []
commands << c.delay(nil,5000)
commands << c.callback(nil)
puts c.sendCommands(commands)

puts "Ready to go!"
for i in 0..10
	s = c.sendCommand(c.checkCallBack())
	puts s
	sleep(1)
end
puts 'Done!'

