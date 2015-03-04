var http = require('http'),
    stringify = require('stringify'),
    EventSource = require('eventsource');


var chairsLogInfo = {
            "53ff6e066667574822262067": "15916cf78641fbabdec9c7766212ad846a9799de", //fan sai
            "53ff6f066667574826352167": "f36986b13669d9499004aed7de5af9c96a871fac", //jeremy
            "53ff68066667574851512367": "e31a1e42deb194ee4cfdfde6bfec16c417cfb935", //vanessa
            "53ff6d066667574815382067": "b142ca27e40c939294795da923ee4a98376ed7be"  //raj
        },
        chairStatus = {},
        waitingChairs = [],
        connectedChairs = [],
        chair,
        chairsNaming = {
            "53ff6e066667574822262067": "Fan Sai", //fan sai
            "53ff6f066667574826352167": "Jeremy", //jeremy
            "53ff68066667574851512367": "Vanessa", //vanessa
            "53ff6d066667574815382067": "Raj"  //raj
        };

for (chair in chairsLogInfo ){
    if(chairsLogInfo.hasOwnProperty(chair)){

        start(chair, chairsLogInfo[chair]);
        chairStatus[chair] = {
            "status": "unconnected",
            "partner": undefined,
            "init": function(){
                this.status = "unconnected";
                this.partner = undefined;
            }
        }
    }
    console.log("Watch chair: ", chair);
    
}


//build connections with spark cores
function start(deviceID, accessToken) {
    var eventSource = new EventSource("https://api.spark.io/v1/devices/" + deviceID + "/events/?access_token=" + accessToken);

    eventSource.addEventListener('error', function(e) {
            console.log("Errored!"); 
        },false);

    eventSource.addEventListener('sdnt/sitdown', sitDownHandler, false);
    eventSource.addEventListener('sdnt/leave', leaveHandler, false);
};


function sitDownHandler(e){
    
    var id = JSON.parse(e.data).coreid;

    if(waitingChairs.indexOf(id) === -1 && chairStatus[id].status === "unconnected"){
        console.log("a");
        chairStatus[id].status = "waiting";
        waitingChairs.push(id);
        console.log("Chair ", id, " added to waiting list");
    }

    matcher();
}


function leaveHandler(e){
    
    var id = JSON.parse(e.data).coreid;

    console.log("Chair ", id, " leaves");

    //if is a waiting chair, remove from the waiting list
    if(chairStatus[id].status === "waiting"){
        waitingChairs = waitingChairs.filter(function(v){
            return v !== id;
        })
        console.log('aaa', waitingChairs);
        chairStatus[id].init();
    //if is a connected chair, disconect it
    }else if(chairStatus[id].status === "connected"){
        disconnect(id);
    }
}

//match waiting chairs in pair
function matcher(){
    console.log('Matcher executed, waiting chairs:', waitingChairs);
    if(waitingChairs.length < 2) return;
    connect(waitingChairs.shift(), waitingChairs.shift());
    if(waitingChairs.length >= 2) matcher();
}


//connect two chairs
function connect(chair1, chair2){
    console.log("Connect ", chair1, " and ", chair2)

    //connect chair1
    chairStatus[chair1].status = "connected";
    chairStatus[chair1].partner = chair2;
    callFn(chair1, "connect", 1);

    //connect chair2
    chairStatus[chair2].status = "connected";
    chairStatus[chair2].partner = chair1;
    callFn(chair2, "connect", 1);
}

function disconnect(chair){
    var partner = chairStatus[chair].partner;
    console.log("Disconnected ", chair);

    //put its partner to "waiting" status,
    if(partner){
        console.log("Disconnected ", chair, "'s partner ", partner);
        waitingChairs.push(partner);
        chairStatus[partner].status = "waiting";
        callFn(partner, "disconnect", 1);
    }

    callFn(chair, "disconnect", 1);
    chairStatus[chair].init();
    matcher();
}



function callFn(chairId, fnName, arg) {
    var requestObj = {
        hostname: "https://api.spark.io/",
        path: "/v1/devices/" + chairId + "/" + fnName + "?access_token=" + chairsLogInfo[chairId],
        method: "POST"
    }

    var req = http.request(requestObj);

    req.write(stringify(arg));
    req.end();


    /*
    $.post( "https://api.spark.io/v1/devices/" + chairId + "/" + fnName + 
                    "?access_token=" + chairsLogInfo[chairId], 
                    { "args": arg });
    */   
}