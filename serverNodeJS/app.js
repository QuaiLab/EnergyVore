// Load the TCP Library
net = require('net');


// INFORMATIONS
// Joystick and robots are numbered
// Each joystick order one robot
// Joystick 4 => Robot 0
// Joystick 5 => Robot 1
// Joystick 6 => Robot 2
// Joystick 7 => Robot 3


// Keep track of the game clients
var clients = [];
var port = 4242;

// Start a TCP Server
net.createServer(function (socket) {

  // Identify this client
  socket.name = socket.remoteAddress + ":" + socket.remotePort;

  // Put this new client in the list
  clients.push(socket);

  // Send a nice welcome message and announce
  socket.write("Welcome " + socket.name + "\n");
  broadcast(socket.name + " joined the server\n", socket);


  // Handle incoming messages from clients.

  socket.on('data', function (data) {


    var dataStr = data.toString();
    console.log(dataStr);

    //If first charater equal Z of datas received, we send ZV to validate the connexion
    if(dataStr[0] == "Z"){


      if(dataStr[1] <= 3) //If second character <= 3, type is ROBOT
      {
        socket.type = "robot";
        socket.number = dataStr[1];
        clients.indexOf(socket).type = socket.type;
        clients.indexOf(socket).number = socket.number;
      }
      else if (dataStr[1] >= 4) //If second character >= 4, type is JOYSTICK
      {
        socket.type = "joystick";
        socket.number = dataStr[1];
        clients.indexOf(socket).type = socket.type;
        clients.indexOf(socket).number = socket.number;
      }

      //Server send ZV at the client, ZV is the code to confirm connexion
      sendMsg('ZV',socket);
      console.log("Synchro "+ socket.type +" number "+ socket.number);

    }else{

      //If receive datas from JOYSTICK
      if(socket.type == "joystick"){

        var numberRobot = socket.number - 4;
        var robotExist = false;

        // if(dataStr == "X9"){
        //   console.log('Allume la LED');
        //   dataStr = "ZD";
        // }
        //
        // if(dataStr == "F9"){
        //   console.log('Eteind la LED');
        //   dataStr = "ZO";
        // }

        //We parse the clients
        clients.forEach(function (client) {
          //We send datas to the right robot
          if (client.number == numberRobot){
            client.write(dataStr);
            console.log("Joystick "+ socket.number +" send "+ dataStr +" to Robot "+ numberRobot);
            robotExist = true;
          }
        });

        //If robot does not exist
        if(robotExist === false){
          console.log("Joystick "+ socket.number +" send "+ dataStr +" to Robot "+ numberRobot + " but he does not exist");
        }

      }
      else //If receive datas from ROBOTS
      {
        console.log("Robot "+ socket.number +" send "+ dataStr +" to Server ");
      }
    }

  });

  // Remove the client from the list when it leaves
  socket.on('end', function () {
    clients.splice(clients.indexOf(socket), 1);
    broadcast(socket.name + " left the chat.\n");
  });

  // Send a message to all clients
  function broadcast(message, sender) {
    clients.forEach(function (client) {
      // Don't want to send it to sender
      if (client === sender) return;
      client.write(message);
    });
    // Log it to the server output too
    process.stdout.write(message+"\n");
  }

  // Send message to a specific client
  function sendMsg(message, receiver){
    receiver.write(message);
  }


}).listen(port);

// Put a friendly message on the terminal of the server.
console.log("Communication server running at port "+ port +"\n");
