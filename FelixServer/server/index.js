var express = require('express');
var app = express();
var os = require('os');
var firebase = require('firebase');
var server = require('http').Server(app);
var io = require('socket.io')(server);
var felixLib = require("./felix");

var existsBoard = false;

try{
  var five = require("johnny-five");
  var chipio = require('chip-io');

  var board = new five.Board({
    io: new chipio()
  });
  existsBoard = true;
}catch(e){
  console.log("Unable to initialize the board.");
}


/******************************************************************************/
/****************************** -- FELIX WEB -- *******************************/
/******************************************************************************/


app.use(express.static('public')); //Se cargaran de forma estatic los documentos contenidos en la carpeta client en nuestro servidor:puerto


var gait = false;
var felix = null;

io.on('connection', function(socket){ //This 'connection' event is what registers the connections to the socket, so that it will run when someone connects
  
  console.log("Client with IP: " + socket.handshake.address + " connecting...");
  socket.emit("legConfiguration", legConfig);
  
  //When data comes from the server I need to update my information and send information to the browser
  felixLegConfiguration.on('value', function(datos){
    legConfig = datos.val();
    socket.emit("legConfiguration", legConfig);
  });

  socket.on("areYouFelix",function(data){
    console.log("Someone questioning if I am Felix??? Of course!");
    socket.emit("connectedWithFelix", "Yes, of course!");
  });

  //The socket what it does is have an open connection where the client can send us events (it runs when client send us changeLegCalibration)
  socket.on("changeLegDegree",function(data){
    var leg = data["l"];
    var hipKnee = data["p"];
    var valueC = parseInt(data["v"]);
    legConfig[leg][hipKnee] = valueC;
    io.sockets.emit("legConfiguration", legConfig);
    console.log("Change " + leg + " " + hipKnee + " to " + valueC + " degrees");
    try{
    	//To update Firebase information
	    var namePart = leg + "/" + hipKnee;
	    var part = {};
	    part[namePart] = valueC;
	    felixLegConfiguration.update(part);  //Should have this form -> felixLegConfiguration.update({"BL/hip": -44});
  	}catch(err){console.log("Unable to store new configuration in firebase");}

    try{
      if(hipKnee=="hip") felix.legs[felix._legIdx(leg)].hip.offset = valueC;
      else if(hipKnee=="knee") felix.legs[felix._legIdx(leg)].knee.offset = valueC;
      felix.calibrateLeg(leg);
    }catch(e){
      console.log("Unable to put the configuration on Felix");
    }
  });


  //The socket what it does is have an open connection where the client can send us events (it runs when client send us changeLegCalibration)
  socket.on("changeHomeLeg",function(data){
    var leg = data["l"];
    var xy = data["p"];
    var valueC = parseInt(data["v"]);
    legConfig[leg][xy] = valueC;
    io.sockets.emit("legConfiguration", legConfig);
    console.log("Change " + leg + " " + xy + " to " + valueC + " degrees");
    try{
      //To update Firebase information
      var namePart = leg + "/" + xy;
      var part = {};
      part[namePart] = valueC;
      felixLegConfiguration.update(part);  //Should have this form -> felixLegConfiguration.update({"BL/x": -44});
    }catch(err){console.log("Unable to store new configuration in firebase");}

    try{
      if(xy=="x") felix.legs[felix._legIdx(leg)].origin.x = valueC;
      else if(xy=="y") felix.legs[felix._legIdx(leg)].origin.y = valueC;
      felix.homeLeg(leg);
    }catch(e){
      console.log("Unable to put the configuration on Felix");
    }
  });


  socket.on("webCommand",function(command){
    treatOrders(command["text"]);
  });

  socket.on("voiceCommand",function(command){
    treatOrders(command["text"]);
  });

  socket.on("playCustomPose",function(thePose){
    console.log("Voy a intentar poner una pose!");
    try{
      felix.setPose(thePose);
      console.log("Pose fijada!");
    }catch(e){console.log("I can't find Felix");} 
  });
  
});


port = 6677;
server.listen(port, '0.0.0.0', function(){

    var interfaces = os.networkInterfaces();
    var addresses = [];
    for (var k in interfaces) {
        for (var k2 in interfaces[k]) {
            var address = interfaces[k][k2];
            if (address.family === 'IPv4' && !address.internal) {
                addresses.push(address.address);
            }
        }
    }

    if(addresses[0]==null) addresses[0] = "localhost";

    console.log("Server running -> http://"+addresses[0]+":"+port);

});


/******************************************************************************/
/************************* -- FELIX FUNCTIONAL -- *****************************/
/******************************************************************************/

// Initialize Firebase (This fragment is given by firebase in the app configuration -> Add firebase to your web app)
var firebaseConfig = {
  apiKey: "AIzaSyC0MOPyGlBPfmHVu_l8zoOaHqv4z7Chu5s",
  authDomain: "felix-config.firebaseapp.com",
  databaseURL: "https://felix-config.firebaseio.com",
  storageBucket: "felix-config.appspot.com",
  messagingSenderId: "210418144318"
};

//We create the firebase object with the configuration provided
firebase.initializeApp(firebaseConfig);

firebase.auth().signInWithEmailAndPassword("maparla@inf.upv.es", "felixSettings").catch(function(error) {
  // Handle Errors here.
  var errorCode = error.code;
  var errorMessage = error.message;
  console.log("");
  console.log("Firebse wrong authentication!");
  console.log(errorMessage);
  console.log("");
});

var legConfig = {"BL":{"hip":0,"knee":0,"x":0,"y":0},"BR":{"hip":0,"knee":0,"x":0,"y":0},"FL":{"hip":0,"knee":0,"x":0,"y":0},"FR":{"hip":0,"knee":0,"x":0,"y":0}};
var config = {};

var boardReady = false;
var configLoaded = false;
var isLoaded = false;
var theBoard;

//We are ready to read the data of our database, the leg-config
var felixLegConfiguration = firebase.database().ref().child('/leg-config');

felixLegConfiguration.once('value', function(datos) { //We will capture the changes of the database in socket connection via on()
	
	legConfig = datos.val();  

	config = {
		granularity:4,
		speed:35,
		geometry:{ femur:44, tibia:74, height:110, step_height:15, step_width:26 },
  		gaits:{'deer_creep':[ //expected order FR, FL, BR, BL
                        [4,2,3,1],
                        [1,3,4,2],
                        [2,4,1,3],
                        [3,1,2,4]
                      ],
          'cat_creep':[
                        [4,1,2,3],
                        [1,2,3,4],
                        [2,3,4,1],
                        [3,4,1,2],
                      ]
        },
  		legs:[  		//expected order FR, FL, BR, BL
               {
                        id:'FR',
                        label:'Front right',
                        origin:{x:legConfig["FR"]["x"],y:legConfig["FR"]["y"]},
                        hip:{ pin:2, offset:legConfig["FR"]["hip"], invert:false },
                        knee:{ pin:3, offset:legConfig["FR"]["knee"], invert:false }
               },
               {
                        id:'FL',
                        label:'Front left',
                        origin:{x:legConfig["FL"]["x"],y:legConfig["FL"]["y"]},
                        hip:{ pin:0, offset:legConfig["FL"]["hip"], invert:true },
                        knee:{ pin:1, offset:legConfig["FL"]["knee"], invert:true }
               },
               {
                        id:'BR',
                        label:'Back right',
                        origin:{x:legConfig["BR"]["x"],y:legConfig["BR"]["y"]},
                        hip:{ pin:6, offset:legConfig["BR"]["hip"], invert:true },
                        knee:{ pin:7, offset:legConfig["BR"]["knee"], invert:true }
               },
               {
                        id:'BL',
                        origin:{x:legConfig["BL"]["x"],y:legConfig["BL"]["y"]},
                        label:'Back left',
                        hip:{ pin:4, offset:legConfig["BL"]["hip"], invert:false },
                        knee:{ pin:5, offset:legConfig["BL"]["knee"], invert:false }
               }
             ]
    };
    console.log("Firebase Felix configuration loaded correctly");
    if(boardReady && !isLoaded){ //If the board is ready
    	isLoaded = true;  //To make sure we only initialize it once
    	felix = new felixLib.Felix(config,five);
	  	felix.stand();
	  	theBoard.repl.inject({
	    	felix:felix
	  	});
    }
    configLoaded = true;
});


function treatOrders(theCommand){
  console.log("La orden es: " + theCommand + "!");
    switch(theCommand){

      case "sientate":
        try{
          felix.holdUp();
          felix.sit();
          console.log("Start sientate!");
        }catch(e){console.log("I can't find Felix");}
        break;

      case "camina":
        try{
          felix.holdUp();
          felix.forward();
          console.log("Start camina!");
        }catch(e){console.log("I can't find Felix");}
        break;

      case "para":
	      try{
          felix.holdUp();
          console.log("Start para!");
        }catch(e){console.log("I can't find Felix");}  
        break;

      case "patita":
	      try{
          felix.holdUp();
	        felix.paw();
          console.log("Start patita!");
        }catch(e){console.log("I can't find Felix");}  
        break;
      
      case "levantate":
	      try{
          felix.holdUp();
          console.log("Start levantate!");
        }catch(e){console.log("I can't find Felix");}  
        break;
      
      case "agachate":
	      try{
          felix.holdUp();
	        felix.getDown();
          console.log("Start agachate!");
        }catch(e){console.log("I can't find Felix");}  
        break;
    }
}


if(existsBoard){
  board.on("ready", function() {
    console.log('');
      console.log('******************************************');
      console.log(' WELCOME TO FELIX ');
      console.log('');
      console.log(' Have fun =)');
      console.log('******************************************');

      boardReady = true;

      if(configLoaded && !isLoaded){ //If the board is ready
        isLoaded = true;  //To make sure we only initialize it once
        felix = new felixLib.Felix(config,five);
        felix.stand();
        this.repl.inject({
          felix:felix
        });
      }else{
        theBoard = this;
      }
  });
}
