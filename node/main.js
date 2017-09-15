
var express = require('express');
var app = express();
var path = require('path');
var http = require('http').Server(app);
var io = require('socket.io')(http);
var SerialPort = require("serialport")//.SerialPort

var socketServer;
var serialPort;
var portName = 'COM6'; //change this to your Arduino port
var sendData = "";

var date = new Date();

app.get('/', function(req, res) {res.sendFile(path.join(__dirname + '/webapp', 'index.html'));});
app.use(express.static(__dirname + '/webapp'));
app.use('/webapp', express.static(__dirname + '/webapp'));
//Socket.io Event handlers
io.on('connection', function(socket) {
    //fetchSettings();
    //sendData();
    
    /*socket.on('fetch settings', function(msg) {fetchSettings();});
    
    socket.on('store settings', function(msg) {
        storeSettings(msg);
        if (msg.litersPerCycle != litersPerCycle) resetFlow();
        loadSettings();
        setTimeout(fetchSettings, 1000);
    });
    
    socket.on('sound stop', function(msg){clearInterval(buzzerInterval); myBuzzer.stopSound();});
    socket.on('reset flow', function(msg){resetFlow();});*/
});

http.listen(3000, function(){console.log('Web server Active listening on *:3000');});
serialListener();
/*var checkDate = checkDate = new Date();
var logMsg = "";

function addMsg(msg){
    if (logMsg == ""){
        logMsg = msg;
    }else{
        logMsg += "<br>" + msg;
    }
}

function getMsg(){
    if (logMsg == ""){
        return "";
    }else{
        return "<span>"+checkDate.toLocaleTimeString()+": </span>"+logMsg;
    }
}*/
var data = {light: 0, lightSys: 0, temp: 0, hum: 0, hi: 0, tempWater: 0, hour: 0};

function sendData(){
    io.emit('send data', data);
}

// Listen to serial port
function serialListener()
{
    var receivedData = "";
    serialPort = new SerialPort(portName, {
        baudrate: 9600,
        // defaults for Arduino serial communication
         dataBits: 8,
         parity: 'none',
         stopBits: 1,
         flowControl: false
    });
 
    serialPort.on("open", function () {
      console.log('open serial communication');
        // Listens to incoming data
        serialPort.on('data', function(data) {
            receivedData += data.toString();
            if (receivedData.indexOf("<CLR>") >= 0){
                console.log(receivedData);
                receivedData = '';
            }else if (receivedData.indexOf('}') >= 0 && receivedData.indexOf('{') >= 0) {
                console.log(receivedData);
                data = JSON.parse(receivedData);//eval(receivedData);//
                io.emit('send data', data);
                receivedData = '';
            }else if (receivedData.indexOf('}') >= 0 && receivedData.indexOf('{M') >= 0) {
                console.log(receivedData);
                //io.emit('send msg', data);
                receivedData = '';
         	}else if(receivedData.indexOf('}') >= 0){
                receivedData = '';
            }else if(receivedData == "T"){
                serialPort.write("T" + Math.floor(Date.now() /1000));
                console.log(date.getTime());
                receivedData = '';
            }
      	});  
    });
    //serialPort.write(data + 'E');  
}
