print("mqtt setup start");
sendMessage("01")
connected=0

m = mqtt.Client("clientid", 30, "A1E-a3nUoARivVvZb5G0ZucYlX4WPeFyTB", "")
 
m:on("message", 
 function(client, topic, data) 
  print("--received message")
  print(topic .. ":" ) 
  if data ~= nil then
    print(data) 
        if( string.match(topic, "s1") ) then    sendMessage("1".. data)
    elseif( string.match(topic, "s2") ) then    sendMessage("2".. data)
    elseif( string.match(topic, "s3") ) then    sendMessage("3".. data)
    elseif( string.match(topic, "s4") ) then    sendMessage("4".. data)
    end 
  end
 end
)
  

function handle_mqtt_error(client, reason) 
 print("--Disconnected - failed reason: " .. reason) 
 connected=0
 tmr.create():alarm(10 * 5000, tmr.ALARM_SINGLE, do_mqtt_connect)
end

function do_mqtt_connect()
    m:connect("things.ubidots.com",1883, 0, 
    function(client)
        print("--Connected to Ubidots!")  
        connected=1
        
            client:subscribe(
            {
              ["/v1.6/devices/sem/s1/lv"]=1,
              ["/v1.6/devices/sem/s2/lv"]=1,
              ["/v1.6/devices/sem/s3/lv"]=1
            },function(client) print("--subscribe success") end) 
    end,
    handle_mqtt_error)
end

do_mqtt_connect()
print("mqtt setup end");

tmr.alarm(0,5000,1,function()
if connected==1 then getValues() end
end)


 
function getValues()
    msg = recvMessage()
    if(msg=="Waiting for net") then 
        sendMessage("01") 
    else
        val={}
        val.v,val.c1,val.c2,val.m = msg:match("([^,]+),([^,]+),([^,]+),([^,]+)")
        val=sjson.encode(val)
        if(val~="[]") then
            print(val)
            m:publish("/v1.6/devices/sem", val, 0, 0, function(client) print("sent") end) 
        else
            print("---*****i2c Communication fail*****") 
        end
    end 
end
