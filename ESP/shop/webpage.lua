function SendHTML(conn,msg,err) 
    local t = {}
table.insert(t,"HTTP/1.1 200 OK")
table.insert(t,"Cache-Control: no-cache, no-store, must-revalidate\r\nPragma: no-cache\r\nExpires: 0\r\nConnection: close\r\nContent-Type: text/html")
table.insert(t,"charset=UTF-8\r\n\r\n")
table.insert(t,"<!DOCTYPE html>\r\n")
table.insert(t,"<html>\r\n<head>")
table.insert(t,"<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/4.1.1/css/bootstrap.min.css\" />")
table.insert(t,"<title>Power Control</title></head>\r\n<body>")
table.insert(t,"<nav class=\"navbar navbar-dark bg-dark navbar-expand-md mb-4\"><a class=\"navbar-brand\" href=\"#\">ESP Power Control and Scheduler </a> </nav>")
table.insert(t,"<div class=\"container\">")
table.insert(t,"<div class=\"alert alert-"..err.."\" role=\"alert\">"..msg.."</div>")
msg=nil
err=nil

table.insert(t,"<div class=\"btn-group\"> ")
table.insert(t,"<input type=\"button\" class=\"btn btn-warning\" value=\"Refresh\" onclick=\"window.location.href='/'\">")
    if(gpio.read(pin1)==1) then table.insert(t,"<input type=\"button\" class=\"btn btn-primary\" value=\"1 Turn Off\" onclick=\"window.location.href='/1?status=0'\">")  
    else                        table.insert(t,"<input type=\"button\" class=\"btn btn-secondary\" value=\"1 Turn On\" onclick=\"window.location.href='/1?status=1'\">"  )
    end

    if(gpio.read(pin2)==1) then table.insert(t,"<input type=\"button\" class=\"btn btn-primary\" value=\"2 Turn Off\" onclick=\"window.location.href='/2?status=0'\">")
    else                        table.insert(t,"<input type=\"button\"  class=\"btn btn-secondary\" value=\"2 Turn On\" onclick=\"window.location.href='/2?status=1'\">")
    end

   if(getSchedule()==1)   then table.insert(t,"<input type=\"button\" class=\"btn btn-success\" value=\"Disable Auto Schedule\" onclick=\"window.location.href='/schedule=0'\">")
   else                        table.insert(t,"<input type=\"button\"  class=\"btn btn-secondary\" value=\"Enable Auto Schedule\" onclick=\"window.location.href='/schedule=1'\">")
   end
table.insert(t,"</div>")

table.insert(t,"<br><br><form method=\"post\" action=\"/cron\">")
table.insert(t,"<div class=\"form-group\"><label>Cron</label>")
table.insert(t,"<textarea class=\"form-control\"  rows=\"15\" name=\"val\" cols=\"100\">")
local s=readCron()
table.insert(t,s)
s=nil
table.insert(t,"</textarea>\r\n</div>")
table.insert(t,"<input type=\"submit\" class=\"btn btn-info\" value=\"Save & Start Schedule\">")
table.insert(t,"</form>")

table.insert(t,"<span id=\"dateinLocal\" > </span> <br>")
table.insert(t,"<span id=\"dateinUTC\" > </span><br>")
table.insert(t,"Boot Time <span id=\"boottime\" > </span><br>")
table.insert(t,"<span> Heap: "..node.heap().."</span><br>")
table.insert(t,"<span> Signal: "..wifi.sta.getrssi().."</span>")
--table.insert(t,"<div class=\"progress\"><div class=\"progress-bar progress-bar-striped progress-bar-animated\" role=\"progressbar\" aria-valuenow=\""..(wifi.sta.getrssi()+100).."\" aria-valuemin=\"0\" aria-valuemax=\"100\" style=\"width: 75%\"></div></div>")
table.insert(t,"</div>")
table.insert(t,"</body>\r\n</html>\r\n<script>")
table.insert(t,"var d = new Date(0); d.setUTCSeconds("..rtctime.get()..")")
table.insert(t,"var b = new Date(0); b.setUTCSeconds("..boottime..")")
table.insert(t,"document.getElementById(\"dateinLocal\").innerHTML = d.toLocaleString(); ")
table.insert(t,"document.getElementById(\"dateinUTC\").innerHTML = d.toUTCString(); ")
table.insert(t,"document.getElementById(\"boottime\").innerHTML = b.toLocaleString(); ")
table.insert(t,"</script>")

local s= table.concat(t ,"\n")
for i=1, #t do
    t[i] = nil        
end
t=nil
conn:send(s)
conn=nil
end
