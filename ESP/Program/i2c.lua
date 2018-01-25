print("i2c setup start");
id=0  
sda=4 --GPIO2  
scl=3 --GPIO0  
i2c.setup(id, sda, scl, i2c.SLOW) 

function sendMessage(i2cMsg)
    print("---sending message: " .. i2cMsg)
    i2c.start(id)  
    i2c.address(id, 2 ,i2c.TRANSMITTER)  
    i2c.write(id,i2cMsg)  
    i2c.stop(id)  
end

function recvMessage()
    i2c.start(id)  
    i2c.address(id, 2 ,i2c.RECEIVER)  
    i2cMsg = i2c.read(id,30)  
    i2c.stop(id)  
    print(i2cMsg)
    ind=string.find(i2cMsg,"%^%~%^")
    if(ind ~= nil)
    then
    i2cMsg = string.sub(i2cMsg, 0, ind-1)
    print(i2cMsg) 
    return i2cMsg
    else
    print("---!!!received null from arduino")
    return ""
    end 
    
end

print("i2c setup end");
