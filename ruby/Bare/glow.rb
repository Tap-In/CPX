require '../cpx.rb'
require './config.rb'

c = CPX.new("http://arduinoproxy.tapinsystems.net:8085/cpx",@user,@deviceid);
#Intense red on pin 6
for i in 0..10
	puts i
	c.sendCommand(c.analogWrite(nil,6,i));
	sleep(1)
end

puts "Done"

