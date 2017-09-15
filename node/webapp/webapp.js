/*
    Aquaponic IoT Controler
    Copyright (C) 2015  Davor Marjanović <aquaponic.controller@gmail.com>
    Licence: GNU GENERAL PUBLIC LICENSE
*/

var socket = io();
var $myLightGauge, $myTempGauge, $myLightSysGauge, $flowRateGauge;
var data;

socket.on('send data', function(msg){
    data = msg;
    $(".progressTitle").text(msg.toString());
    $myLightGauge.changeValue(data.light);
    $myLightSysGauge.changeValue(data.lightSys);
    $myTempGauge.changeValue(data.temp);
    $flowRateGauge.changeValue(msg.flowRate);
    $(".flowCycle progress").val(msg.flowLiters);
    $(".flowCycle .progressTxt").text(msg.flowLiters +" / "+ $(".flowCycle progress").attr("max"));
    /*$("#flowTimeLast").text(getMinSec(msg.flowTimeLast));
    $("#flowTimeAvg").text(getMinSec(msg.flowTimeAvg));
    $("#flowTimeCnt").text(msg.flowTimeCnt);
    relay("relay1", msg.relay1);
    relay("relay2", msg.relay2);
    relay("relay3", msg.relay3);
    relay("relay4", msg.relay4);
    if(msg.event != ""){
        $('#events').prepend($('<li>'+msg.event+'</li>'));
    }*/
});

socket.on('updateData', function(msg){
	$(".progressTitle").text(msg);
});

$( function () {
    $myLightGauge = $("div#lightGauge").dynameter({
    	width: 200,
    	label: 'Light',
    	value: 50,
    	min: 0,
    	max: 100,
    	unit: '%',
    	regions: {
    		50: 'warn',
    		90: 'error'
    	}
    });

    $myTempGauge = $("div#tempGauge").dynameter({
    	width: 200,
    	label: 'Temp.',
    	value: 20,
    	min: 0.0,
    	max: 40.0,
    	unit: '&deg;C',
    	regions: {
    		0: 'warn',
    		10: 'normal',
    		30: 'error'
    	}
    });
    
    $myLightSysGauge = $("div#lightSysGauge").dynameter({
    	width: 200,
    	label: 'Light',
    	value: 50,
    	min: 0,
    	max: 100,
    	unit: '%',
    	regions: {
    		50: 'warn',
    		90: 'error'
    	}
    });

    $flowRateGauge = $("div#flowRateGauge").dynameter({
    	width: 200,
    	label: 'Flow Rate',
    	value: 0,
    	min: 0.0,
    	max: 5.0,
    	unit: 'LPM',
    	regions: {
    		0: 'error',
            1: 'warn',
    		2: 'normal'
    	}
    });
});