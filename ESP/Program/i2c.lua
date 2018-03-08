print("i2c setup start");
id =0 
sda=4 --GPIO2  
scl=3 --GPIO0  
i2c.setup(id, sda, scl, i2c.SLOW) 

function sendMessage(i2cMsg)
    print("--i2c sending message: " .. i2cMsg)
    i2c.start(id)  
    i2c.address(id, 2 ,i2c.TRANSMITTER) 
    i2c.write(id,i2cMsg)  
    i2c.stop(id)  
end

function recvMessage()
    i2c.start(id)  
    i2c.address(id, 2 ,i2c.RECEIVER)  
    i2cMsg = i2c.read(id,30) --read 30 bytes
    i2c.stop(id)  
    
    ind=string.find(i2cMsg,"%^%~%^") --position of ^~^
    if(ind ~= nil) --if index is not null
    then
        i2cMsg = string.sub(i2cMsg, 0, ind-1) --extract message till ^~^
        print("--i2c received message: "..i2cMsg) 
        return i2cMsg
    else
        print("--i2c received empty!!! / comm. fail")
        return ""
    end 
end
print("i2c setup end");
