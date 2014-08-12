require '../cpx.rb'
require './config.rb'

c = CPX.new(@host,@user,@deviceid);
commands = []
commands << c.digitalWrite("loop",6,0)
commands << c.delay(nil,250)
commands << c.digitalWrite(nil,6,1)
commands << c.delay(nil,250)
commands << c.jump(nil,"loop",10)
commands << c.digitalWrite(nil,6,0)
commands << c.digitalWrite(nil,4,0)
c.sendCommands(commands);

puts "Done"

