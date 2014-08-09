require '../cpx.rb'
require './config.rb'

#trigger on button press A15 or return after 30 seconds, use 0 instead of 30000 for infinite wait.

c = CPX.new("http://arduinoproxy.tapinsystems.net:8085/cpx",@user,@deviceid)
puts c.sendCommand(c.trigger(nil,69,0));
puts 'Done!'

