// Initialize legConfig
var legConfig = {"BL":{"hip":0,"knee":0,"x":0,"y":0},"BR":{"hip":0,"knee":0,"x":0,"y":0},"FL":{"hip":0,"knee":0,"x":0,"y":0},"FR":{"hip":0,"knee":0,"x":0,"y":0}};
var customPose = []
// Los angulos se conforman como sigue:   FL      -      FR      -     BL       -     BR
customPose.push({cmd:'pose',angles:[{hip:0,knee:0},{hip:0,knee:0},{hip:0,knee:0},{hip:0,knee:0}]});  // Desde la vista superior
var ipformat = /^(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}):(\d{1,5})$/;
var noValidIp = "Please insert a correct IP and port";
var socket = null;

$( document ).ready(function() {
  $(".main").hide();
  $(".buttonVibrate").vibrate("medium");

  $("#chipIp").keyup(function(event){
    if(event.keyCode == 13){ //When press Enter when focusing the input
      if($(this).val()=="fIrEbAsE") connectToFirebase();
      else if($(this).val().match(ipformat)) connectToServer($(this).val());
      else errorOnConnect(noValidIp);
    }
  });

  $(".submitIp").on("click",function(){
  	  if($(this).val()=="fIrEbAsE") connectToFirebase();
      else if($("#chipIp").val().match(ipformat)) connectToServer($("#chipIp").val());
      else errorOnConnect(noValidIp);
  });

  //When the document its loaded, nothing is selected -> set input values to 0
  $(".inputOffset").val(0);
  $(".inputOffsetHomeLeg").val(0);

  $(".main").onepage_scroll({
     sectionContainer: "section",     // sectionContainer accepts any kind of selector in case you don't want to use section
     easing: "ease",                  // Easing options accepts the CSS3 easing animation such "ease", "linear", "ease-in",
                                      // "ease-out", "ease-in-out", or even cubic bezier value such as "cubic-bezier(0.175, 0.885, 0.420, 1.310)"
     animationTime: 1000,             // AnimationTime let you define how long each section takes to animate
     pagination: false,                // You can either show or hide the pagination. Toggle true for show, false for hide.
     updateURL: false,                // Toggle this true if you want the URL to be updated automatically when the user scroll to each page.
     beforeMove: function(index) {},  // This option accepts a callback function. The function will be called before the page moves.
     afterMove: function(index) {},   // This option accepts a callback function. The function will be called after the page moves.
     loop: false,                     // You can have the page loop back to the top/bottom when the user navigates at up/down on the first/last page.
     keyboard: true,                  // You can activate the keyboard controls
     responsiveFallback: false,        // You can fallback to normal page scroll by defining the width of the browser in which
                                      // you want the responsive fallback to be triggered. For example, set this to 600 and whenever
                                      // the browser's width is less than 600, the fallback will kick in.
     direction: "vertical"            // You can now define the direction of the One Page Scroll animation. Options available are "vertical" and "horizontal". The default value is "vertical".
  });

  //We don't want a clickeable leg calibration input value
  $(".inputLegsCalibration").click(function() {
    $(this).blur();
  });

   /* -- WHEN A BUTTON OF THE SAME GROUP IS SELECTED, CHANGE ITS APPEARANCE -- */
  $(".buttonConfigGroup").click(function() {
    if($(this).parents("section").first().attr('id')=="legsConfiguration"){
      $(this).siblings().removeClass("secSelected"); //Remove the highlight class from all the buttons from the same group (siblings)
      $(this).addClass("secSelected");  //Add a highlight to de button clicked
    }else if($(this).parents("section").first().attr('id')=="homeLegConfiguration"){
      $(this).siblings().removeClass("secSelectedHomeLeg"); //Remove the highlight class from all the buttons from the same group (siblings)
      $(this).addClass("secSelectedHomeLeg");  //Add a highlight to de button clicked
    }else if($(this).parents("section").first().attr('id')=="customPoseConfiguration"){
      $(this).siblings().removeClass("secSelectedCustomPose"); //Remove the highlight class from all the buttons from the same group (siblings)
      $(this).addClass("secSelectedCustomPose");  //Add a highlight to de button clicked
    }
  });

  //This function check if we have selected two parts of the same section 
  //If 2 selected -> set the value for the combiantion in the correct input
  $("button").click(function(){
    //After a buttons it's clicked, he will lose the focus
    $(this).blur(); //To remove the focus from the input (we will add a class)
    if($(".secSelected").length==2){ //When we choosed the Leg and the hip/knee
      var leg = $(".secSelected").first().text().toUpperCase(); //Take which leg
      var hk = $(".secSelected:last").text().toLowerCase(); //Take hip or knee
      $(".inputOffset").val(legConfig[leg][hk]); //Set the value to the stored value
    }
    if($(".secSelectedHomeLeg").length==2){ //When we choosed the Leg and the x/y
      var leg = $(".secSelectedHomeLeg").first().text().toUpperCase(); //Take which leg
      var xy = $(".secSelectedHomeLeg:last").text().toLowerCase(); //Take x or y
      $(".inputOffsetHomeLeg").val(legConfig[leg][xy]); //Set the value to the stored value
    }
    if($(".secSelectedCustomPose").length==2 && !$(this).hasClass("saveCustomPose")){ //When we choosed the Leg and the x/y
      var leg = parseInt($(".secSelectedCustomPose").first().data("ref")); //Take which leg
      var hk = $(".secSelectedCustomPose:last").text().toLowerCase(); //Take x or y
      $(".inputOffsetCustomPose").val(customPose[0]["angles"][leg][hk]); //Set the value to the stored value
    }
  });

  /* -- FUNCTIONS FOR INCREASE AND DECREASE INPUT VALUE -- */
  $(".increase").click(function() {
    if($(".secSelected").length==2){ //When we choosed the Leg and the hip/knee
      var leg = $(".secSelected").first().text().toUpperCase(); //Take which leg
      var hk = $(".secSelected:last").text().toLowerCase(); //Take hip or knee
      $(this).siblings(".inputOffset").val((parseInt($(this).siblings(".inputOffset").val())+1));
      legConfig[leg][hk] = $(".inputOffset").val(); //Assign to the object representing the selected configuration the assigned value
      try{socket.emit("changeLegDegree",{l:leg, p:hk, v:$(".inputOffset").val()});}
      catch(e){ console.log("Can't send the value to the server"); }
    }else{
      $('#myModal').modal('show');
    }
  });

  $(".decrease").click(function() {
    if($(".secSelected").length==2){ //When we choosed the Leg and the hip/knee
      var leg = $(".secSelected").first().text().toUpperCase(); //Take which leg
      var hk = $(".secSelected:last").text().toLowerCase(); //Take hip or knee
      $(this).siblings(".inputOffset").val((parseInt($(this).siblings(".inputOffset").val())-1));
      legConfig[leg][hk] = $(".inputOffset").val(); //Assign to the object representing the selected configuration the assigned value
      try{socket.emit("changeLegDegree",{l:leg, p:hk, v:$(".inputOffset").val()});}
      catch(e){ console.log("Can't send the value to the server"); }
    }else{
      $('#myModal').modal('show');
    }
  });

  /* -- FUNCTIONS FOR INCREASE AND DECREASE INPUT VALUE -- */
  $(".increaseHomeLeg").click(function() {
    if($(".secSelectedHomeLeg").length==2){ //When we choosed the Leg and the hip/knee
      var leg = $(".secSelectedHomeLeg").first().text().toUpperCase(); //Take which leg
      var xy = $(".secSelectedHomeLeg:last").text().toLowerCase(); //Take hip or knee
      $(this).siblings(".inputOffsetHomeLeg").val((parseInt($(this).siblings(".inputOffsetHomeLeg").val())+1));
      legConfig[leg][xy] = $(".inputOffsetHomeLeg").val(); //Assign to the object representing the selected configuration the assigned value
      try{socket.emit("changeHomeLeg",{l:leg, p:xy, v:$(".inputOffsetHomeLeg").val()});}
      catch(e){ console.log("Can't send the value to the server"); }
    }else{
      $('#myModalHomeLeg').modal('show');
    }
  });

  $(".decreaseHomeLeg").click(function() {
    if($(".secSelectedHomeLeg").length==2){ //When we choosed the Leg and the hip/knee
      var leg = $(".secSelectedHomeLeg").first().text().toUpperCase(); //Take which leg
      var xy = $(".secSelectedHomeLeg:last").text().toLowerCase(); //Take x or y
      $(this).siblings(".inputOffsetHomeLeg").val((parseInt($(this).siblings(".inputOffsetHomeLeg").val())-1));
      legConfig[leg][xy] = $(".inputOffsetHomeLeg").val(); //Assign to the object representing the selected configuration the assigned value
      try{socket.emit("changeHomeLeg",{l:leg, p:xy, v:$(".inputOffsetHomeLeg").val()});}
      catch(e){ console.log("Can't send the value to the server"); }
    }else{
      $('#myModalHomeLeg').modal('show');
    }
  });

  $(".saveCustomPose").click(function() {
    if($(".secSelectedCustomPose").length==2){ //When we choosed the Leg and the hip/knee
      var leg = parseInt($(".secSelectedCustomPose").first().data("ref")); //Take which leg
      var hk = $(".secSelectedCustomPose:last").text().toLowerCase(); //Take x or y
      customPose[0]["angles"][leg][hk] = parseInt($(this).siblings(".inputOffsetCustomPose").val());
    }else{
      $('#myModalCustomPose').modal('show');
    }
  });

  $(".playCustomPose").click(function() {
    console.log(customPose[0]);
    try{socket.emit("playCustomPose",customPose);}
    catch(e){ console.log("Can't send the value to the server"); }
  });


  /* -- FUNCTION SEND -> Web Commands! -- */
  $(".webCommands").click(function(){
    try{socket.emit("webCommand",{text:$(this).text().toLowerCase()});}
    catch(e){ console.log("Can't send the value to the server"); }
  });

});

var attemptsToReconnect = 3;

function connectToServer(myIp){
  //alert(myIp);
  $(".felixIp").hide();
  $(".loader").show();

  var attemptsDone = 0;
  var connected = false;

  var site = 'http://'+myIp;

  try{

    socket = io.connect(site, {
        'reconnection': true,
        'reconnectionDelay': 1000,
        'reconnectionDelayMax' : 1250,
        'reconnectionAttempts': attemptsToReconnect
    });

    /*
    his.socket.on("connect") to catch connection events
    this.socket.on("disconnect") to catch disconnection events
    this.socket.on("connect_failed") to catch failed connection attempts
    this.socket.io.on("connect_error") to catch if the server is offline.
    255.255.255.255 -> ALWAYS FAILS
    */
    

    socket.on("connect",function(){
      try{
        socket.emit("areYouFelix","Are you Felix?");
        setTimeout(function(){  //After 2 seconds, if we have not response from Felix -> Return to the input page
          if(!connected){
            console.log("He is not Felix :O"); 
            $(".loader").hide();
            $(".felixIp").show();
            errorOnConnect("Felix is not responding on this IP");
          }
        }, 2000);
      }
      catch(e){ 
        console.log("He is not Felix :O"); 
        $(".loader").hide();
        $(".felixIp").show();
        errorOnConnect("This is not the Felix IP");
      }
    });

    socket.on("connectedWithFelix",function(){
      connected = true;
      $(".loader").hide();
      $(".main").moveTo(1);
      $(".main").show();
    });

    socket.on("connect_error",function(){
      attemptsDone++;
      console.log("Error trying to connect to " + site);
      if(attemptsDone==attemptsToReconnect+1){
        $(".loader").hide();
        $(".felixIp").show();
        errorOnConnect("Cannot connect to " + site);
      }
    });

    //The server will sen us the configuration saved on index.js
    socket.on("legConfiguration", function(configuration){
      
      legConfig["FR"]["hip"] = configuration["FR"]["hip"];
      legConfig["FR"]["knee"] = configuration["FR"]["knee"];
      legConfig["FR"]["x"] = configuration["FR"]["x"];
      legConfig["FR"]["y"] = configuration["FR"]["y"];
      legConfig["FL"]["hip"] = configuration["FL"]["hip"];
      legConfig["FL"]["knee"] = configuration["FL"]["knee"];
      legConfig["FL"]["x"] = configuration["FL"]["x"];
      legConfig["FL"]["y"] = configuration["FL"]["y"];
      legConfig["BR"]["hip"] = configuration["BR"]["hip"];
      legConfig["BR"]["knee"] = configuration["BR"]["knee"];
      legConfig["BR"]["x"] = configuration["BR"]["x"];
      legConfig["BR"]["y"] = configuration["BR"]["y"];
      legConfig["BL"]["hip"] = configuration["BL"]["hip"];
      legConfig["BL"]["knee"] = configuration["BL"]["knee"];
      legConfig["BL"]["x"] = configuration["BL"]["x"];
      legConfig["BL"]["y"] = configuration["BL"]["y"];

      //We could be changing the configuration with multiple devices, this will synch all devices
      if($(".secSelected").length==2){ //When we choosed the Leg and the hip/knee
        var leg = $(".secSelected").first().text().toUpperCase(); //Take which leg
        var hk = $(".secSelected:last").text().toLowerCase(); //Take hip or knee
        $(".inputOffset").val(legConfig[leg][hk]);
      }
      if($(".secSelectedHomeLeg").length==2){ //When we choosed the Leg and the x/y
          var leg = $(".secSelectedHomeLeg").first().text().toUpperCase(); //Take which leg
          var xy = $(".secSelectedHomeLeg:last").text().toLowerCase(); //Take hip or knee
          $(".inputOffsetHomeLeg").val(legConfig[leg][xy]); //Reset the value to the stored value
      }
    });

  }catch(e){
    console.log("Unable to find io!");
    console.log(e);
    $(".loader").hide();
    $(".felixIp").show();
    errorOnConnect("Unable to find io!");
  }
}


function connectToFirebase(){
  //alert(myIp);
  $(".felixIp").hide();
  $(".loader").show();

  var firebaseLoaded = false;

  setTimeout(function(){  //After 2 seconds, if we have not response from Felix -> Return to the input page
    if(!firebaseLoaded){
      $(".loader").hide();
      $(".felixIp").show();
      errorOnConnect("Cannot connect with Firebase");
    }
  }, 6250);

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

  var authenticationError = false;
  firebase.auth().signInWithEmailAndPassword("maparla@inf.upv.es", "felixSettings").catch(function(error) {
    // Handle Errors here.
    authenticationError = true;
    var errorCode = error.code;
    var errorMessage = error.message;
    console.log("");
    console.log("Firebse wrong authentication!");
    console.log(errorMessage);
    console.log("");
    $(".loader").hide();
    $(".felixIp").show();
    errorOnConnect(errorMessage);
  });

  if(!authenticationError){
    //We are ready to read the data of our database, the leg-config
    var felixLegConfiguration = firebase.database().ref().child('/leg-config');
    felixLegConfiguration.once('value', function(datos) { //We will capture the changes of the database in socket connection via on()	
      firebaseLoaded = true;
      legConfig = datos.val();
      console.log("Firebase Felix configuration loaded correctly");
      //console.log(legConfig);
      $(".loader").hide();
    });
  }

}

function errorOnConnect(message){
  if($(".errorMessage").length>0)
    $(".errorMessage").text(message);
  else
    $(".inputIpGroup").parent().append("<p class='errorMessage'>"+message+"</p>");
}

