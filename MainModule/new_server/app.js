//Hint: npx kill-port 3000 kills the process in port 3000



const express = require('express')
const fs = require('fs');
const upload = require("express-fileupload");

const app = express()
const port = 3000
const MYSQLClient = require("mysql")

var latestToolIndex = 0;
var logsString = "";

app.use(upload());

// MySQL connection setup
const con = MYSQLClient.createConnection({
    host: "localhost",
    user: "root",
    password: "password"
})

con.connect(function(err)
{
if(err) throw err;

console.log("Connected to MySQL server");
});

app.listen(port, () => {
    console.log(`Server listening at http://localhost:${port}`)
})

app.param('requestingIP', function(req, res, next, requestingIP) {
    req.requestingIP = requestingIP;
    next();
});

function eventStringUpdater(eventName, eventDate, eventIssuer)
{
	logsString += "Event: " + eventName + "\nDate: " + eventDate + "\nIssuer: " + eventIssuer + "\n\n";
}

function eventTableUpdater(req, eventName)
{
	var currentdate = new Date(); 
    var datetime = "" + currentdate.getDate() + "/"
                + (currentdate.getMonth()+1)  + "/" 
                + currentdate.getFullYear() + " @ "  
                + currentdate.getHours() + ":"  
                + currentdate.getMinutes() + ":" 
                + currentdate.getSeconds();
	var sql = "INSERT INTO data.events (event_name, event_date, event_issuer) VALUES ('" + eventName+ "', '" + datetime+ "', '" + req.requestingIP+ "');";
	con.query(sql, function(err, result) {
    if (err)
    {
        console.log("Could not insert information into the database");
        throw err;
    }
    })
    eventStringUpdater(eventName, datetime, req.requestingIP);
}

app.get("/notifyOperatorForFall/:requestingIP", function(req, res) 
{		
    eventTableUpdater(req, "Worker Fall");
	res.status(200).send();
});

app.get("/downloadAudio/:requestingIP", function(req, res) 
{
	eventTableUpdater(req, "Audio Download");
	res.download('audio_files/audioToBeSent.mp3');
});

app.get("/getTool1Layout/:requestingIP", function(req, res) 
{
	eventTableUpdater(req, "Tool 1 Layout Request");
	res.download('tool_image_files/LatestDrillLocationsLayout.png');	
});

app.get("/getTool2Layout/:requestingIP", function(req, res) 
{
	eventTableUpdater(req, "Tool 2 Layout Request");
	res.download('tool_image_files/LatestGrinderLocationsLayout.png');	
});

app.get("/getTool3Layout/:requestingIP", function(req, res) 
{
	eventTableUpdater(req, "Tool 3 Layout Request");
	res.download('tool_image_files/LatestKeyboardLocationsLayout.png');	
});

app.get("/getTool1Real/:requestingIP", function(req, res) 
{
	eventTableUpdater(req, "Tool 1 Real Request");
	res.download('tool_image_files/LatestDrillLocationsSquared.png');	
});

app.get("/getTool2Real/:requestingIP", function(req, res) 
{
	eventTableUpdater(req, "Tool 2 Real Request");
	res.download('tool_image_files/LatestGrinderLocationsSquared.png');	
});

app.get("/getTool3Real/:requestingIP", function(req, res) 
{
	eventTableUpdater(req, "Tool 3 Real Request");
	res.download('tool_image_files/LatestKeyboardLocationsSquared.png');	
});

app.post("/addAudio/:requestingIP", function(req, res) 
{
	eventTableUpdater(req, "Audio Upload");
	if(req.files){
  		var file_name = req.files.upload_file.name;
        var file = req.files.upload_file;
        //console.log(file_name);		
      	file.mv("audio_files/" + file_name, (err)=>{
            if(err)
            {
                console.log("An error occurred while uploading.");
                res.status(400).send();
                throw err;
            }
            else
            {
                var sql = "INSERT INTO data.audios (audio_name) VALUES ('" + file_name+ "');";
                con.query(sql, function(err, result) {
                if (err)
                {
                    console.log("Could not insert information into the database");
                    throw err;
                }
                console.log("Saved " + file_name+ " successfully");
                //console.log(result);
                })
                res.status(200).send();
            }
          });          
      }
});

app.post("/addImage/:requestingIP", function(req, res) 
{
	eventTableUpdater(req, "Image Upload");
	if(req.files){
  		var file_name1 = req.files.upload_file1.name;
        var file1 = req.files.upload_file1;
        var file_name2 = req.files.upload_file2.name;
        var file2 = req.files.upload_file2;
        var file_name3 = req.files.upload_file3.name;
        var file3 = req.files.upload_file3;
      	file1.mv("image_files/" + file_name1, (err)=>{
            if(err)
            {
                console.log("An error occurred while uploading.");
                res.status(400).send();
                throw err;
            }
            else
            {
                var sql = "INSERT INTO data.images (image_name) VALUES ('" + file_name1+ "');";

                con.query(sql, function(err, result) {
                if (err)
                {
                    console.log("Could not insert information into the database");
                    throw err;
                }
                console.log("Saved " + file_name1+ " successfully");
                //console.log(result);
                })

                res.status(200).send();
            }
          });   
          file2.mv("image_files/" + file_name2, (err)=>{
            if(err)
            {
                console.log("An error occurred while uploading.");
                res.status(400).send();
                throw err;
            }
            else
            {
                var sql = "INSERT INTO data.images (image_name) VALUES ('" + file_name2+ "');";

                con.query(sql, function(err, result) {
                if (err)
                {
                    console.log("Could not insert information into the database");
                    throw err;
                }
                console.log("Saved " + file_name2+ " successfully");
                //console.log(result);
                })

                res.status(200).send();
            }
          }); 
          file3.mv("image_files/" + file_name3, (err)=>{
            if(err)
            {
                console.log("An error occurred while uploading.");
                res.status(400).send();
                throw err;
            }
            else
            {
                var sql = "INSERT INTO data.images (image_name) VALUES ('" + file_name3+ "');";

                con.query(sql, function(err, result) {
                if (err)
                {
                    console.log("Could not insert information into the database");
                    throw err;
                }
                console.log("Saved " + file_name3+ " successfully");
                })

                res.status(200).send();
            }
          });                          
      }
});


app.get("/getLogs/:requestingIP", function(req, res) 
{
	res.send(logsString);	
});










