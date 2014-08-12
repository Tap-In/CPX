require '../cpx.rb'
require './config.rb'

# LED at pin 6 blinks. When A15 (69) read is not 0 LED at 6 is turned off

c = CPX.new(@host,@user,@deviceid);
commands = []
commands << c.digitalWrite("loop",6,1)
commands << c.digitalRead(nil,69)
commands << c.delay(nil,250);
commands << c.digitalWrite(nil,6,0)
commands << c.delay(nil,250);
commands << c.testR(nil,"loop","eq",0)
c.sendCommands(commands)
