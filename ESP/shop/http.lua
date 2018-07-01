print("http begin")
encoded=""
gpio.write(pin1, readGpio(pin1))
gpio.write(pin2, readGpio(pin2))

srv=net.createServer(net.TCP,1) 

srv:listen(81,function(con) 
con:on("receive",function(conn,payload) 
    enduser_setup.stop()
    print("receive",node.heap()) 
    encoded=payload
    local payload=string.sub(payload,4,15)
        if string.find(payload, "/1?status=1",0,true)  then   saveGpio(pin1,1)  SendHTML(conn,"1 on ","success") 
    elseif string.find(payload, "/1?status=0",0,true)  then   saveGpio(pin1,0)  SendHTML(conn,"1 off","warning") 
    elseif string.find(payload, "/2?status=1",0,true)  then   saveGpio(pin2,1)  SendHTML(conn,"2 on ","success") 
    elseif string.find(payload, "/2?status=0",0,true)  then   saveGpio(pin2,0)  SendHTML(conn,"2 off","warning") 
    elseif string.find(payload, "/schedule=0",0,true)  then   saveSchedule(0)   SendHTML(conn,"Schedule Off","warning") 
    elseif string.find(payload, "/schedule=1",0,true)  then   saveSchedule(1)   SendHTML(conn,"Schedule On","success") 
    elseif string.find(payload, "/restart",0,true)     then  node.restart()
    elseif string.find(payload, "/cron",0,true)  then    
        status, ret= xpcall  ( verifyandStartCron, function( err )  print( "ERROR:", err ) return err end)  
       
            if(status) then SendHTML(conn,"Okay","success") 
            else            SendHTML(conn,ret,"danger")
            end
    else  
        SendHTML(conn,"Welcome","primary")
    end      
    payload=nil
    encoded=nil

    end)   
con:on("sent", function(c) 
    c:close()  
    collectgarbage()    
    print("closed",node.heap())  
    end)

end)
 
if(getSchedule()==1) then startCron() end
print("http end")
